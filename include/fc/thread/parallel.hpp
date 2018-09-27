/*
 * Copyright (c) 2018 The BitShares Blockchain, and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <fc/thread/task.hpp>
#include <fc/asio.hpp>

namespace fc {

   namespace detail {
      template<typename Task>
      class parallel_completion_handler {
         public:
            parallel_completion_handler( Task* task ) : _task(task) {}
            void operator()() { _task->run(); }
         private:
            Task* _task;
      };
   }

   /**
    *  Calls function <code>f</code> in a separate thread and returns a future
    *  that can be used to wait on the result.
    *
    *  @param f the operation to perform
    */
   template<typename Functor>
   auto do_parallel( Functor&& f, const char* desc FC_TASK_NAME_DEFAULT_ARG ) -> fc::future<decltype(f())> {
      typedef decltype(f()) Result;
      typedef typename fc::deduce<Functor>::type FunctorType;
      fc::task<Result,sizeof(FunctorType)>* tsk =
         new fc::task<Result,sizeof(FunctorType)>( fc::forward<Functor>(f), desc );
      fc::future<Result> r(fc::shared_ptr< fc::promise<Result> >(tsk,true) );
      fc::asio::default_io_service().post( detail::parallel_completion_handler<fc::task<Result,sizeof(FunctorType)>>( tsk ) );
      return r;
   }
}
