#pragma once
#include <unordered_map>
#include "BasicUsageEnvironment0.hh"

#include "event_dispatcher_timer.hpp"
#include "net/socket_poll.hpp"

namespace uvxx { namespace rtsp { namespace details 
{
    class _uvxx_task_scheduler : public BasicTaskScheduler0
    {
    public:
        explicit _uvxx_task_scheduler(unsigned maxSchedulerGranularity);

        virtual ~_uvxx_task_scheduler();

        _uvxx_task_scheduler(const _uvxx_task_scheduler&) = delete;

        _uvxx_task_scheduler& operator=(const _uvxx_task_scheduler&) = delete;

    protected:
        static void schedulerTickTask(void* client_data);

        void schedulerTickTask();

    protected:
        virtual void doEventLoop(char* watchVariable);

        virtual void SingleStep(unsigned max_delay_time) override;

        virtual void setBackgroundHandling(int socket, int condition_set, BackgroundHandlerProc* handler_proc, void* client_data) override;

        virtual void moveSocketHandling(int old_socket, int new_socket) override;

        virtual TaskToken scheduleDelayedTask(int64_t microseconds, TaskFunc* proc, void* client_data) override;

        void set_next_timeout();

        virtual void unscheduleDelayedTask(TaskToken& prevTask) override;

        virtual void triggerEvent(EventTriggerId eventTriggerId, void* clientData = nullptr) override;

        class socket_handler_descriptor
        {
        public:
            socket_handler_descriptor(int socket, 
                                      int condition_set, 
                                      BackgroundHandlerProc* handler_proc, 
                                      void* client_data);

            ~socket_handler_descriptor();
      
            socket_handler_descriptor(const socket_handler_descriptor&) = delete;

            socket_handler_descriptor& operator=(const socket_handler_descriptor&) = delete;

            socket_handler_descriptor(socket_handler_descriptor&& rhs);

            socket_handler_descriptor& operator=(socket_handler_descriptor&& rhs);

            void set_condition_set(int condition_set);

            void set_socket(int socket);

            void set_handler(BackgroundHandlerProc* handler_proc, void* client_data);

        private:
            void start_poll();

            void poll_callback(int status, net::socket_poll_event events);

        private:
            int _socket;

            int _condition_set;

            void* _client_data;

            net::socket_poll _poller;

            BackgroundHandlerProc* _handler_proc;
        };

      private:
        void on_timer_tick(event_dispatcher_timer* sender);

    protected:
        unsigned fMaxSchedulerGranularity;

        event_dispatcher_timer _timer;

        std::unordered_map<int, socket_handler_descriptor> _handlers;
    };
}}}