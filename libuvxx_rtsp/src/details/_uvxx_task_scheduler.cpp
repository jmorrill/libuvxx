#include "BasicUsageEnvironment.hh"
#include "HandlerSet.hh"
#include "details/_uvxx_task_scheduler.h"

#include <stdio.h>

#ifndef MILLION
#define MILLION 1000000
#endif

using namespace uvxx::net;
using namespace uvxx::rtsp::details;

_uvxx_task_scheduler* _uvxx_task_scheduler::createNew(unsigned maxSchedulerGranularity)
{
    return new _uvxx_task_scheduler(maxSchedulerGranularity);
}

_uvxx_task_scheduler::_uvxx_task_scheduler(unsigned maxSchedulerGranularity)
  : fMaxSchedulerGranularity(maxSchedulerGranularity)
{
    if (maxSchedulerGranularity > 0)
    {
        //schedulerTickTask(); // ensures that we handle events frequently
    }

    _timer.timeout_set(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::microseconds(maxSchedulerGranularity)));

    /* Hook the tick event */
    _timer.tick_event() += std::bind(&_uvxx_task_scheduler::on_timer_tick, this, std::placeholders::_1);

    _timer.start();
}

_uvxx_task_scheduler::~_uvxx_task_scheduler() 
{

}

void _uvxx_task_scheduler::doEventLoop(char* watchVariable)
{
    throw std::exception("doEventLoop not supported.  Use uvxx event_dispatcher");
}

void _uvxx_task_scheduler::on_timer_tick(uvxx::event_dispatcher_timer* sender)
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
    int64_t millis = (timeToDelay.seconds() * 1000ull) + (timeToDelay.useconds() / 1000);

    if (fTriggersAwaitingHandling != 0)
    {
        millis = 0;
    }

    _timer.timeout_set(std::chrono::milliseconds(millis));
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
    ((_uvxx_task_scheduler*)clientData)->schedulerTickTask();
}

void _uvxx_task_scheduler::schedulerTickTask() 
{
    scheduleDelayedTask(fMaxSchedulerGranularity, schedulerTickTask, this);
}

void _uvxx_task_scheduler::SingleStep(unsigned maxDelayTime) 
{
    DelayInterval const& timeToDelay = fDelayQueue.timeToNextAlarm();
    struct timeval tv_timeToDelay;

    tv_timeToDelay.tv_sec = timeToDelay.seconds();
    tv_timeToDelay.tv_usec = timeToDelay.useconds();
  
    const long MAX_TV_SEC = MILLION;

    if (tv_timeToDelay.tv_sec > MAX_TV_SEC) 
    {
        tv_timeToDelay.tv_sec = MAX_TV_SEC;
    }

    // Also check our "maxDelayTime" parameter (if it's > 0):
    if (maxDelayTime > 0 &&
        (tv_timeToDelay.tv_sec > (long)maxDelayTime / MILLION ||
        (tv_timeToDelay.tv_sec == (long)maxDelayTime / MILLION &&
         tv_timeToDelay.tv_usec > (long)maxDelayTime % MILLION))) 
    {
        tv_timeToDelay.tv_sec = maxDelayTime / MILLION;
        tv_timeToDelay.tv_usec = maxDelayTime % MILLION;
    }

    if (fTriggersAwaitingHandling != 0)
    {
        if (fTriggersAwaitingHandling == fLastUsedTriggerMask)
        {
            // Common-case optimization for a single event trigger:
            fTriggersAwaitingHandling &=~ fLastUsedTriggerMask;
            if (fTriggeredEventHandlers[fLastUsedTriggerNum] != nullptr) 
            {
                (*fTriggeredEventHandlers[fLastUsedTriggerNum])(fTriggeredEventClientDatas[fLastUsedTriggerNum]);
            }
        } 
        else 
        {
            // Look for an event trigger that needs handling (making sure that we make forward progress through all possible triggers):
            unsigned i = fLastUsedTriggerNum;
            EventTriggerId mask = fLastUsedTriggerMask;

            do 
            {
                i = (i+1) % MAX_NUM_EVENT_TRIGGERS;

                mask >>= 1;

                if (mask == 0) 
                {
                    mask = 0x80000000;
                }

                if ((fTriggersAwaitingHandling&mask) != 0) 
                {
                    fTriggersAwaitingHandling &=~ mask;

                    if (fTriggeredEventHandlers[i] != nullptr)
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

    // Also handle any delayed event that may have come due.
    fDelayQueue.handleAlarm();
}

void _uvxx_task_scheduler::setBackgroundHandling(int socket, int condition_set, BackgroundHandlerProc* handler_proc, void* client_data) 
{
    if (socket < 0)
    {
        return;
    }

    if (condition_set == 0)
    {
        _handlers.erase(socket);
        return;
    }

    if (_handlers.find(socket) == _handlers.end())
    {
        socket_handler_descriptor socket_handler(socket, condition_set, handler_proc, client_data);
        _handlers.emplace(socket, std::move(socket_handler));
    }
    else
    {
        auto& handler = _handlers.at(socket);
        handler.set_condition_set(condition_set);
    }
}

void _uvxx_task_scheduler::moveSocketHandling(int old_socket, int new_socket) 
{
    if (old_socket < 0 || new_socket < 0)
    {
        return;
    }

    if (_handlers.find(old_socket) == _handlers.end())
    {
        auto handler = std::move(_handlers.at(old_socket));

        handler.set_socket(new_socket);

        _handlers.erase(old_socket);

        _handlers.emplace(new_socket, std::move(handler));
    }
}

_uvxx_task_scheduler::socket_handler_descriptor::socket_handler_descriptor(int socket, int condition_set, BackgroundHandlerProc* handler_proc, void* client_data) : 
    _poller(socket)
{
    _socket = socket;
    _condition_set = condition_set;
    _handler_proc = handler_proc;
    _client_data = client_data;

    start_poll();
}

_uvxx_task_scheduler::socket_handler_descriptor::socket_handler_descriptor(socket_handler_descriptor&& rhs) :
    _poller(std::move(rhs._poller))
{
    _client_data = rhs._client_data;
    rhs._client_data = nullptr;

    _socket = rhs._socket;
    rhs._socket = 0;

    _condition_set = rhs._condition_set;
    rhs._condition_set = 0;

    _handler_proc = rhs._handler_proc;
    rhs._handler_proc = nullptr;

    start_poll();
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
    _condition_set = condition_set;
    start_poll();
}

void _uvxx_task_scheduler::socket_handler_descriptor::set_socket(int socket)
{
    _socket = socket;
    _poller = socket_poll(_socket);
    start_poll();
}

_uvxx_task_scheduler::socket_handler_descriptor& _uvxx_task_scheduler::socket_handler_descriptor::operator=(socket_handler_descriptor&& rhs)
{
    return std::move(rhs);
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

    _poller.set_callback(std::bind(&socket_handler_descriptor::poll_callback, this, std::placeholders::_1, std::placeholders::_2));

    _poller.start(events);
}

void _uvxx_task_scheduler::socket_handler_descriptor::poll_callback(int status, socket_poll_event events)
{
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

    _handler_proc(_client_data, mask);
}
