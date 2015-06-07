#pragma once
#include "uvxx.hpp"
#include "BasicUsageEnvironment0.hh"
#include <map>

namespace uvxx { namespace rtsp { namespace details 
{
    class _uvxx_task_scheduler : public BasicTaskScheduler0
    {
    public:
        static _uvxx_task_scheduler* createNew(unsigned maxSchedulerGranularity = 1000000 /*microseconds*/ );

        virtual ~_uvxx_task_scheduler();

    protected:
        _uvxx_task_scheduler(unsigned maxSchedulerGranularity);

        static void schedulerTickTask(void* client_data);

        void schedulerTickTask();

    protected:
        virtual void doEventLoop(char* watchVariable);

        virtual void SingleStep(unsigned max_delay_time);

        virtual void setBackgroundHandling(int socket, int condition_set, BackgroundHandlerProc* handler_proc, void* client_data);

        virtual void moveSocketHandling(int old_socket, int new_socket);

        virtual TaskToken scheduleDelayedTask(int64_t microseconds, TaskFunc* proc, void* client_data);

        void set_next_timeout();

        virtual void unscheduleDelayedTask(TaskToken& prevTask);

        virtual void triggerEvent(EventTriggerId eventTriggerId, void* clientData = nullptr);

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

            void set_condition_set(int condition_set);

            void set_socket(int socket);

            socket_handler_descriptor& operator=(socket_handler_descriptor&& rhs);

        private:
            void start_poll();

            void poll_callback(int status, uvxx::net::socket_poll_event events);

        private:
            int _socket;
            int _condition_set;
            void* _client_data;
            uvxx::net::socket_poll _poller;
            BackgroundHandlerProc* _handler_proc;
        };

      private:
        void on_timer_tick(uvxx::event_dispatcher_timer* sender);

    protected:
        unsigned fMaxSchedulerGranularity;

        uvxx::event_dispatcher_timer _timer;

        std::map<int, socket_handler_descriptor> _handlers;
    };
}}}