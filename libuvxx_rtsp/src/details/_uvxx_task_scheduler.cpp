#include <stdio.h>
#include <algorithm>
#include "details/_uvxx_task_scheduler.hpp"

#ifndef MILLION
#define MILLION 1000000
#endif

using namespace uvxx;
using namespace uvxx::net;
using namespace uvxx::rtsp::details;

_uvxx_task_scheduler::_uvxx_task_scheduler(unsigned maxSchedulerGranularity)
    : BasicTaskScheduler0(),
    fMaxSchedulerGranularity(maxSchedulerGranularity)
{
    _timer.timeout_set(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::microseconds(maxSchedulerGranularity)));

    _timer.tick_event() += std::bind(&_uvxx_task_scheduler::on_timer_tick, this, std::placeholders::_1);

    _timer.start();
}

_uvxx_task_scheduler::~_uvxx_task_scheduler()
{

}

void _uvxx_task_scheduler::doEventLoop(char* /*watchVariable*/)
{
    throw std::exception(/*"doEventLoop not supported.  Use uvxx event_dispatcher"*/);
}

void _uvxx_task_scheduler::on_timer_tick(event_dispatcher_timer* /*sender*/)
{
    SingleStep(fMaxSchedulerGranularity);

    set_next_timeout();
}

TaskToken _uvxx_task_scheduler::scheduleDelayedTask(int64_t microseconds, TaskFunc* proc, void* clientData)
{
    auto taskToken = BasicTaskScheduler0::scheduleDelayedTask(microseconds, proc, clientData);

    set_next_timeout();

    return taskToken;
}

void _uvxx_task_scheduler::set_next_timeout()
{
    DelayInterval const& timeToDelay = fDelayQueue.timeToNextAlarm();

    int64_t milliseconds = (timeToDelay.seconds() * 1000ll) + (timeToDelay.useconds() / 1000ll);

    if (milliseconds < 0)
    {
        milliseconds = 0;
    }

    _timer.timeout_set(std::chrono::milliseconds(milliseconds));
}

void _uvxx_task_scheduler::unscheduleDelayedTask(TaskToken& prevTask)
{
    BasicTaskScheduler0::unscheduleDelayedTask(prevTask);

    set_next_timeout();
}

void _uvxx_task_scheduler::triggerEvent(EventTriggerId eventTriggerId, void* clientData)
{
    BasicTaskScheduler0::triggerEvent(eventTriggerId, clientData);

    set_next_timeout();
}

void _uvxx_task_scheduler::schedulerTickTask(void* clientData)
{
    static_cast<_uvxx_task_scheduler*>(clientData)->schedulerTickTask();
}

void _uvxx_task_scheduler::schedulerTickTask()
{
    scheduleDelayedTask(fMaxSchedulerGranularity, schedulerTickTask, this);
}

void _uvxx_task_scheduler::SingleStep(unsigned /* maxDelayTime */)
{
    if (fTriggersAwaitingHandling != 0)
    {
        if (fTriggersAwaitingHandling == fLastUsedTriggerMask)
        {
            /* Common-case optimization for a single event trigger */
            fTriggersAwaitingHandling &= ~fLastUsedTriggerMask;

            if (fTriggeredEventHandlers[fLastUsedTriggerNum])
            {
                (*fTriggeredEventHandlers[fLastUsedTriggerNum])(fTriggeredEventClientDatas[fLastUsedTriggerNum]);
            }
        }
        else
        {
            /* Look for an event trigger that needs handling
            making sure that we make forward progress through all possible triggers */
            unsigned i = fLastUsedTriggerNum;

            EventTriggerId mask = fLastUsedTriggerMask;

            do
            {
                i = (i + 1) % MAX_NUM_EVENT_TRIGGERS;

                mask >>= 1;

                if (mask == 0)
                {
                    mask = 0x80000000;
                }

                if ((fTriggersAwaitingHandling & mask) != 0)
                {
                    fTriggersAwaitingHandling &= ~mask;

                    if (fTriggeredEventHandlers[i])
                    {
                        (*fTriggeredEventHandlers[i])(fTriggeredEventClientDatas[i]);
                    }

                    fLastUsedTriggerMask = mask;

                    fLastUsedTriggerNum = i;

                    break;
                }

            } while (i != fLastUsedTriggerNum);
        }
    }

    /* handle any delayed event that may have come due. */
    fDelayQueue.handleAlarm();

    prune_dead_pollers();
}


void _uvxx_task_scheduler::prune_dead_pollers()
{
    for (auto it = std::begin(_handlers); it != std::end(_handlers);)
    {
        if (it->second.has_disabled_poll_timed_out())
        {
            printf("pruned socket poller for %d", it->second.socket());

            it = _handlers.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void _uvxx_task_scheduler::setBackgroundHandling(int socket, int condition_set, BackgroundHandlerProc* handler_proc, void* client_data)
{
    if (socket < 0)
    {
        return;
    }

    auto handler_iterator = _handlers.find(socket);

    bool found = handler_iterator != _handlers.end();

    if (condition_set == 0 || !handler_proc)
    {
        if (found)
        {
            handler_iterator->second.set_condition_set(0);
        }

        return;
    }

    if (!found)
    {
        _handlers.emplace(std::piecewise_construct,
                          std::forward_as_tuple(socket),
                          std::forward_as_tuple(socket, condition_set, handler_proc, client_data));
    }
    else
    {
        auto& handler = handler_iterator->second;

        handler.set_handler(condition_set, handler_proc, client_data);

        handler.set_condition_set(condition_set);
    }
}

void _uvxx_task_scheduler::moveSocketHandling(int old_socket, int new_socket)
{
    if (old_socket < 0 || new_socket < 0)
    {
        return;
    }

    auto iterator = _handlers.find(old_socket);

    if (iterator != _handlers.end())
    {
        auto handler = std::move(iterator->second);

        handler.set_socket(new_socket);

        _handlers.erase(iterator);

        _handlers.emplace(new_socket, std::move(handler));
    }
}

_uvxx_task_scheduler::socket_handler_descriptor::socket_handler_descriptor(int socket,
    int condition_set,
    BackgroundHandlerProc* handler_proc,
    void* client_data) :
    _socket(socket),
    _condition_set(condition_set),
    _client_data(client_data),
    _poller(socket),
    _handler_proc(handler_proc),
    _is_socket_valid(true),
    _poll_disabled_since(std::chrono::high_resolution_clock::time_point::max())
{
    _poller.set_callback(std::bind(&socket_handler_descriptor::poll_callback, this, std::placeholders::_1, std::placeholders::_2));

    start_poll();
}

_uvxx_task_scheduler::socket_handler_descriptor& _uvxx_task_scheduler::socket_handler_descriptor::operator=(socket_handler_descriptor&& rhs)
{
    if (this != &rhs)
    {
        _client_data = rhs._client_data;

        rhs._client_data = nullptr;

        _socket = rhs._socket;

        rhs._socket = 0;

        _condition_set = rhs._condition_set;

        rhs._condition_set = 0;

        _handler_proc = rhs._handler_proc;

        rhs._handler_proc = nullptr;

        _poller = std::move(rhs._poller);

        _is_socket_valid = rhs._is_socket_valid;

        rhs._is_socket_valid = false;

        _poller.set_callback(std::bind(&socket_handler_descriptor::poll_callback, this, std::placeholders::_1, std::placeholders::_2));
    }

    return *this;
}

_uvxx_task_scheduler::socket_handler_descriptor::socket_handler_descriptor(socket_handler_descriptor&& rhs)
{
    *this = std::move(rhs);
}

_uvxx_task_scheduler::socket_handler_descriptor::~socket_handler_descriptor()
{
    if (_poller)
    {
        _poller.stop();
    }
}

void _uvxx_task_scheduler::socket_handler_descriptor::set_condition_set(int condition_set)
{
    if (condition_set == _condition_set)
    {
        return;
    }

    _condition_set = condition_set;

    if(_condition_set == 0)
    {
        stop_poll();
        
        return;
    }

    start_poll();
}

void _uvxx_task_scheduler::socket_handler_descriptor::set_handler(int condition_set, BackgroundHandlerProc* handler_proc, void* client_data)
{
    bool had_handler = _handler_proc != nullptr;

    bool was_polling = _poller.is_polling();

    _handler_proc = handler_proc;

    _client_data = client_data;

    _condition_set = condition_set;

    if (_condition_set == 0)
    {
        stop_poll();

        return;
    }

    if ((_handler_proc && !had_handler) || (_handler_proc && !was_polling))
    {
        start_poll();
    }

    if (!_handler_proc)
    {
        stop_poll();
    }
}

int _uvxx_task_scheduler::socket_handler_descriptor::socket()
{
    return _socket;
}

void _uvxx_task_scheduler::socket_handler_descriptor::set_socket(int socket)
{
    _socket = socket;

    _poller = socket_poll(_socket);

    start_poll();
}

void _uvxx_task_scheduler::socket_handler_descriptor::start_poll()
{
    socket_poll_event events = static_cast<socket_poll_event>(0);

    if (_condition_set & SOCKET_READABLE)
    {
        events |= socket_poll_event::Readable;
    }

    if (_condition_set & SOCKET_WRITABLE)
    {
        events |= socket_poll_event::Writeable;
    }

    _poller.start(events);
}

void _uvxx_task_scheduler::socket_handler_descriptor::poll_callback(int status, socket_poll_event events)
{
    if (!_handler_proc)
    {
        return;
    }

    int mask = 0;

    if ((events & socket_poll_event::Readable) == socket_poll_event::Readable)
    {
        mask |= SOCKET_READABLE;
    }

    if ((events & socket_poll_event::Writeable) == socket_poll_event::Writeable)
    {
        mask |= SOCKET_WRITABLE;
    }

    if (status)
    {
        mask |= SOCKET_EXCEPTION;
    }

    if (_condition_set & SOCKET_EXCEPTION ||
        _condition_set & SOCKET_WRITABLE ||
        _condition_set & SOCKET_READABLE)
    {
        _handler_proc(_client_data, mask);
    }
}

void _uvxx_task_scheduler::socket_handler_descriptor::stop_poll()
{
    _poll_disabled_since = std::chrono::high_resolution_clock::now();

    _poller.stop();
}

bool _uvxx_task_scheduler::socket_handler_descriptor::has_disabled_poll_timed_out()
{
    if(_poller.is_polling())
    {
        return false;
    }

    auto now = std::chrono::high_resolution_clock::now();

    auto time_since_start = std::chrono::duration_cast<std::chrono::seconds>(now - _poll_disabled_since);

    static std::chrono::seconds zero_seconds(0);

    if(time_since_start < zero_seconds)
    {
        return false;
    }

    static std::chrono::seconds seconds_to_timeout(10);

    if(time_since_start > seconds_to_timeout)
    {
        return true;
    }
    else
    {
        return false;
    }
}