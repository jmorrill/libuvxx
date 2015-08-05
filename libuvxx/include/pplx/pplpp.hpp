#pragma once
#include <memory>
#include <chrono>
#include <functional>
#include <type_traits>
#include "pplxtasks.h"
#include "event_dispatcher_timer.hpp"

namespace uvxx { namespace pplx { namespace details
{
    
    /* Optimized interative_task_impl (http://pplpp.codeplex.com/SourceControl/latest#include/impl/pplppimplshare.h).  
       This version uses move semantics and std::bind to avoid copies and ref counting of captured objects.  A cleaner
       version can probably be made using generalized lambda capture, but many compilers don't support yet.
       Note that vanilla pplx does not have movable task_completion events or task_continuation_context. 
       Unlike the original interative_task_impl, this only exits the loop by exception */
    inline static void iterative_task_impl(
        task_completion_event<void> finished, 
        std::function<task<bool>()> body,
        cancellation_token          ct, 
        cancellation_token_source   cts, 
        task_continuation_context   context = task_continuation_context::use_default())
    {
        if (ct.is_canceled())
        {
            cts.cancel();
            return;
        }

        auto body_result_task = body();

        std::function<void(task<bool>)> b = std::bind([](
            task<bool>                  previous,
            task_completion_event<void> finished_,
            std::function<task<bool>()> body_,
            cancellation_token          ct_,
            cancellation_token_source   cts_,
            task_continuation_context   context_) mutable
        {
            try
            {
                auto continue_result = previous.get();

                if (!continue_result)
                {
                    finished_.set();
                    return;
                }

                iterative_task_impl(std::move(finished_),
                    std::move(body_),
                    std::move(ct_),
                    std::move(cts_),
                    std::move(context_));
            }
            catch (task_canceled)
            {
                cts_.cancel();
            }
            catch (...)
            {
                finished_.set_exception(std::current_exception());
            }

        }, std::placeholders::_1, std::move(finished), std::move(body), std::move(ct), std::move(cts), std::move(context));

        body_result_task.then(b, context);
    }
}
    /// <summary>
    ///     Creates a task iteratively execute user Functor. During the process, each new iteration will be the continuation of the
    ///     last iteration's returning task, and the process will keep going on until throws and exception.
    /// </summary>
    /// <param name="body">
    ///     The user Functor used as loop body. It is required to throw and exception, which used as predictor that decides
    ///     whether the loop needs to be continued.
    /// </param>
    /// <param name="ct">
    ///     The cancellation token linked to the iterative task.
    /// </param>
    /// <returns>
    ///     The task that will perform the asynchronous iterative execution.
    /// </returns>
    /// <remarks>
    ///     This function dynamically creates a long chain of continuations by iteratively concating tasks created by user Functor <paramref name="body"/>,
    ///     The iteration will not stop until the result of the returning task from user Functor <paramref name="body"/> throws an exception.
    /// </remarks>
    inline task<void> create_iterative_task(
        std::function<task<bool>()> body,
        task_continuation_context   context = task_continuation_context::use_default(),
        cancellation_token          ct = cancellation_token::none())
    {
        task_completion_event<void> finished;
        cancellation_token_source cts;


        std::function<void()> runnable = std::bind([](
                                         task_completion_event<void> finished_, 
                                         std::function<task<bool>()> body_,
                                         cancellation_token          ct_,
                                         cancellation_token_source   cts_,
                                         task_continuation_context   context_) mutable
        {
            try
            {
                details::iterative_task_impl(finished_, 
                                             std::move(body_), 
                                             std::move(ct_), 
                                             std::move(cts_), 
                                             std::move(context_));
            }
            catch(...)
            {
                finished_.set_exception(std::current_exception());
            }

        }, finished, std::move(body), std::move(ct), cts, context);

        if (context == task_continuation_context::use_current())
        {
            runnable();
        }
        else
        {
            create_task(std::move(runnable));
        }

        return task<void>(finished, cts.get_token());
    }

    inline task<void> create_timer_task(std::chrono::milliseconds timeout)
    {
        event_dispatcher_timer timer;
       
        return timer.delay(timeout);
    }

    template<typename T>
    inline task<void> create_for_loop_task(T from_inclusive, T to_exclusive, std::function<task<void>(T)> loop_function)
    {
        auto from_ = std::make_shared<T>(from_inclusive);

        return create_iterative_task([=]
        {
            auto task_ = loop_function(*from_.get());

            bool finished = false;

            ++*from_.get();

            if (*from_.get() >= to_exclusive)
            {
                finished = true;
            }

            return task_.then([=]
            {
                if(finished)
                {
                    return false;
                }
                
                return true;
            });
        });
    }

}}
