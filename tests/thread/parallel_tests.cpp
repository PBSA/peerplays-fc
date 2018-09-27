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

#include <boost/test/unit_test.hpp>

#include <fc/thread/parallel.hpp>

BOOST_AUTO_TEST_SUITE(parallel_tests)

BOOST_AUTO_TEST_CASE( do_nothing_parallel )
{
   std::vector<fc::future<void>> results;
   results.reserve( 20 );
   for( size_t i = 0; i < results.capacity(); i++ )
      results.push_back( fc::do_parallel( [i] () { std::cout << i << ","; } ) );
   for( auto& result : results )
      result.wait();
   std::cout << "\n";
}

BOOST_AUTO_TEST_CASE( do_something_parallel )
{
   struct result {
      boost::thread::id thread_id;
      int               call_count;
   };

   std::vector<fc::future<result>> results;
   results.reserve( 20 );
   boost::thread_specific_ptr<int> tls;
   for( size_t i = 0; i < results.capacity(); i++ )
      results.push_back( fc::do_parallel( [i,&tls] () {
         if( !tls.get() ) { tls.reset( new int(0) ); }
         result res = { boost::this_thread::get_id(), (*tls.get())++ };
         return res;
      } ) );

   std::map<boost::thread::id,std::vector<int>> results_by_thread;
   for( auto& res : results )
   {
      result r = res.wait();
      results_by_thread[r.thread_id].push_back( r.call_count );
   }

   BOOST_CHECK( results_by_thread.size() > 1 ); // require execution by more than 1 thread
   for( auto& pair : results_by_thread )
   {  // check that thread_local_storage counter works
      std::sort( pair.second.begin(), pair.second.end() );
      for( size_t i = 0; i < pair.second.size(); i++ )
         BOOST_CHECK_EQUAL( i, pair.second[i] );
   }
}

BOOST_AUTO_TEST_SUITE_END()
