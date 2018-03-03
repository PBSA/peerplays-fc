#include <boost/test/unit_test.hpp>

#include <fc/exception/exception.hpp>
#include <fc/io/buffered_iostream.hpp>
#include <fc/io/fstream.hpp>
#include <fc/io/iostream.hpp>
#include <fc/io/json.hpp>
#include <fc/io/sstream.hpp>

#include <fstream>

BOOST_AUTO_TEST_SUITE(json_tests)

static void replace_some( std::string& str )
{
   for( size_t i = 0; i < str.length(); i++ )
      if( str[i] == '\1' ) str[i] = '\0';
      else if( str[i] == '\'' ) str[i] = '"';
}

static void test_fail_string( const std::string& str )
{
   try {
      fc::json::from_string( str );
      BOOST_FAIL( "json::from_string('" + str + "') failed" );
   } catch( const fc::parse_error_exception& ) { // ignore, ok
   } catch( const fc::eof_exception& ) { // ignore, ok
   } FC_CAPTURE_LOG_AND_RETHROW( ("json::from_string failed")(str) )
}

static void test_fail_stream( const std::string& str )
{
   fc::temp_file file( fc::temp_directory_path(), true );
   {
      std::fstream init( file.path().to_native_ansi_path(), std::fstream::out | std::fstream::trunc );
      init.write( str.c_str(), str.length() );
      init.close();
   }
   try {
      fc::istream_ptr in( new fc::ifstream( file.path() ) );
      fc::buffered_istream bin( in );
      fc::json::from_stream( bin );
      BOOST_FAIL( "json::from_stream('" + str + "') failed using ifstream" );
   } catch( const fc::parse_error_exception& ) { // ignore, ok
   } catch( const fc::eof_exception& ) { // ignore, ok
   } FC_CAPTURE_LOG_AND_RETHROW( ("json::from_stream failed using ifstream")(str) )
   try {
      fc::istream_ptr in( new fc::stringstream( str ) );
      fc::buffered_istream bin( in );
      fc::json::from_stream( bin );
      BOOST_FAIL( "json::from_stream('" + str + "') failed using stringstream" );
   } catch( const fc::parse_error_exception& ) { // ignore, ok
   } catch( const fc::eof_exception& ) { // ignore, ok
   } FC_CAPTURE_LOG_AND_RETHROW( ("json::from_stream failed using stringstream")(str) )
}

static void test_fail_file( const std::string& str )
{
   fc::temp_file file( fc::temp_directory_path(), true );
   {
      std::fstream init( file.path().to_native_ansi_path(), std::fstream::out | std::fstream::trunc );
      init.write( str.c_str(), str.length() );
      init.close();
   }
   try {
      fc::json::from_file( file.path() );
      BOOST_FAIL( "json::from_file('" + str + "') failed using" );
   } catch( const fc::parse_error_exception& ) { // ignore, ok
   } catch( const fc::eof_exception& ) { // ignore, ok
   } FC_CAPTURE_LOG_AND_RETHROW( ("json::from_file failed")(str) )
}

BOOST_AUTO_TEST_CASE(imbalanced_test)
{
   std::vector<std::string> tests
   { // for easier handling and better readability, in the following test
     // strings ' is used instead of " and \1 instead of \0
      "{",
      "{'",
      "{'}",
      "{'a'",
      "{'a':",
      "{'a':5",
      "[",
      "['",
      "[']",
      "[ 13",
      "' end",
      "{ 13: }",
      "{\1",
      "{\1}",
      "{'\1",
      "{'\1}",
      "{'a'\1",
      "{'a'\1}",
      "{'a': \1",
      "{'a': \1}",
      "[\1",
      "[\1]",
      "['\1",
      "['\1]",
      "[ 13\1",
      "[ 13\1]",
      "' end\1"
   };

   for( std::string test : tests )
   {
      replace_some( test );
      test_fail_string( test );
      test_fail_stream( test );
      test_fail_file( test );
   }
}


BOOST_AUTO_TEST_SUITE_END()
