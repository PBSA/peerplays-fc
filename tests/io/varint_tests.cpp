#include <boost/test/unit_test.hpp>

#include <fc/io/json.hpp>
#include <fc/io/raw.hpp>
#include <fc/io/varint.hpp>

BOOST_AUTO_TEST_SUITE(varint_tests)

#define UINT_LENGTH (1 + 1 + 1 + 1 + 1 + 1 + 2 + 2 + 2 + 3 + 3 + 3 + 4 + 4 + 4 + 5 + 5)
static const std::string EXPECTED_UINTS( "\020" // 16 = length of array
                                         "\0\1\2\020\177\200\1\200\2\377\177"
                                         "\200\200\1\200\200\2\377\377\177"
                                         "\200\200\200\1\200\200\200\2"
                                         "\377\377\377\177\200\200\200\200\1"
                                         "\252\325\252\325\012", UINT_LENGTH );
static const std::vector<fc::unsigned_int> TEST_U = {
                                                 0, // \0
                                                 1, // \1
                                                 2, // \2
                                              0x10, // \020
                                              0x7f, // \177
                                              0x80, // \200\1
                                             0x100, // \200\2
                                            0x3fff, // \377\177
                                            0x4000, // \200\200\1
                                            0x8000, // \200\200\2
                                          0x1fffff, // \377\377\177
                                          0x200000, // \200\200\200\1
                                          0x400000, // \200\200\200\2
                                         0xfffffff, // \377\377\377\177
                                        0x10000000, // \200\200\200\200\1
                                        0xaaaaaaaa  // \252\325\252\325\012
   };
BOOST_AUTO_TEST_CASE( test_unsigned )
{
   const std::vector<char> packed_u = fc::raw::pack<std::vector<fc::unsigned_int>>( TEST_U, 3 );
   BOOST_CHECK_EQUAL( UINT_LENGTH, packed_u.size() );
   BOOST_CHECK_EQUAL( EXPECTED_UINTS, std::string( packed_u.data(), packed_u.size() ) );
   std::vector<fc::unsigned_int> unpacked_u;
   fc::raw::unpack<std::vector<fc::unsigned_int>>( packed_u, unpacked_u, 3 );
   BOOST_CHECK_EQUAL( TEST_U.size(), unpacked_u.size() );
   for( size_t i = 0; i < TEST_U.size(); i++ )
      BOOST_CHECK_EQUAL( TEST_U[i].value, unpacked_u[i].value );

   const std::string json_u = fc::json::to_string(fc::variant( TEST_U, 3 ));
   BOOST_CHECK_EQUAL( "[0,1,2,16,127,128,256,16383,16384,32768,2097151,2097152,4194304,268435455,268435456,2863311530]", json_u );
   std::vector<fc::unsigned_int> unjson_u = fc::json::from_string( json_u ).as<std::vector<fc::unsigned_int>>( 3 );
   BOOST_CHECK_EQUAL( TEST_U.size(), unjson_u.size() );
   for( size_t i = 0; i < TEST_U.size(); i++ )
      BOOST_CHECK_EQUAL( TEST_U[i].value, unjson_u[i].value );
}

BOOST_AUTO_TEST_SUITE_END()
