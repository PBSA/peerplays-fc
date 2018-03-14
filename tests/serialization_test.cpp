#include <boost/test/unit_test.hpp>
#include <fc/log/logger.hpp>

#include <fc/container/flat.hpp>
#include <fc/io/raw.hpp>

namespace fc { namespace test {

   struct item;
   inline bool operator == ( const item& a, const item& b );
   inline bool operator < ( const item& a, const item& b );

   struct item_wrapper
   {
      item_wrapper() {}
      item_wrapper(item&& it) { v.reserve(1); v.insert( it ); }
      boost::container::flat_set<struct item> v;
   };

   inline bool operator == ( const item_wrapper& a, const item_wrapper& b )
   { return ( std::tie( a.v ) == std::tie( b.v ) ); }
   inline bool operator < ( const item_wrapper& a, const item_wrapper& b )
   { return ( std::tie( a.v ) < std::tie( b.v ) ); }

   struct item
   {
      item(int32_t lvl = 0) : level(lvl) {}
      item(item_wrapper&& wp, int32_t lvl = 0) : level(lvl), w(wp) {}
      int32_t      level;
      item_wrapper w;
   };

   inline bool operator == ( const item& a, const item& b )
   { return ( std::tie( a.level, a.w ) == std::tie( b.level, b.w ) ); }
   inline bool operator < ( const item& a, const item& b )
   { return ( std::tie( a.level, a.w ) < std::tie( b.level, b.w ) ); }

   class class_item;

   class class_item_wrapper
   {
      public:
         class_item_wrapper() {}
         class_item_wrapper(class_item&& it) { v.reserve(1); v.emplace_back( it ); }
         std::vector<class class_item> v;
   };
   inline bool operator == ( const class_item_wrapper& a, const class_item_wrapper& b )
   { return ( std::tie( a.v ) == std::tie( b.v ) ); }

   class class_item
   {
      public:
         class_item(int32_t lvl = 0) : level(lvl) {}
         class_item(class_item_wrapper&& wp, int32_t lvl = 0) : level(lvl), w(wp) {}
         int32_t            level;
         class_item_wrapper w;
   };
   inline bool operator == ( const class_item& a, const class_item& b )
   { return ( std::tie( a.level, a.w ) == std::tie( b.level, b.w ) ); }

   template<typename Stream>
   void operator >> ( Stream& s, class_item_wrapper& w )
   {
      fc::raw::unpack( s, w.v );
   }
   template<typename Stream>
   void operator << ( Stream& s, const class_item_wrapper& w )
   {
      fc::raw::pack( s, w.v );
   }

   template<typename Stream>
   void operator >> ( Stream& s, class_item& it )
   {
      fc::raw::unpack( s, it.level );
      fc::raw::unpack( s, it.w );
   }
   template<typename Stream>
   void operator << ( Stream& s, const class_item& it )
   {
      fc::raw::pack( s, it.level );
      fc::raw::pack( s, it.w );
   }

} } // namespace fc::test

namespace fc {
   template<> struct get_typename<fc::test::class_item>  { static const char* name()  { return "class_item";  } };
   template<> struct get_typename<fc::test::class_item_wrapper>  { static const char* name()  { return "class_item_wrapper";  } };
}

FC_REFLECT( fc::test::item_wrapper, (v) );
FC_REFLECT( fc::test::item, (level)(w) );

BOOST_AUTO_TEST_SUITE(fc_serialization)

BOOST_AUTO_TEST_CASE( nested_objects_test )
{ try {

   auto create_nested_object = []( uint32_t level )
   {
      ilog( "Creating nested object with ${lv} level(s)", ("lv",level) );
      fc::test::item nested;
      for( uint32_t i = 1; i <= level; i++ )
      {
         if( i % 100 == 0 )
            ilog( "Creating level ${lv}", ("lv",i) );
         fc::test::item_wrapper wp( std::move(nested) );
         nested = fc::test::item( std::move(wp), i );
      }
      return nested;
   };

   auto create_nested_class_object = []( uint32_t level )
   {
      ilog( "Creating nested class object with ${lv} level(s)", ("lv",level) );
      fc::test::class_item nested;
      for( uint32_t i = 1; i <= level; i++ )
      {
         if( i % 100 == 0 )
            ilog( "Creating level ${lv}", ("lv",i) );
         fc::test::class_item_wrapper wp( std::move(nested) );
         nested = fc::test::class_item( std::move(wp), i );
      }
      return nested;
   };

   // 100 levels, should be allowed
   {
      auto nested = create_nested_object( 100 );

      std::stringstream ss;

      BOOST_TEST_MESSAGE( "About to pack." );
      fc::raw::pack( ss, nested );

      BOOST_TEST_MESSAGE( "About to unpack." );
      fc::test::item unpacked;
      fc::raw::unpack( ss, unpacked );

      BOOST_CHECK( unpacked == nested );
   }

   // 150 levels, by default packing will fail
   {
      auto nested = create_nested_object( 150 );

      std::stringstream ss;

      BOOST_TEST_MESSAGE( "About to pack." );
      BOOST_CHECK_THROW( fc::raw::pack( ss, nested ), fc::assert_exception );
   }

   // 150 levels and allow packing, unpacking will fail
   {
      auto nested = create_nested_object( 150 );

      std::stringstream ss;

      BOOST_TEST_MESSAGE( "About to pack." );
      fc::raw::pack( ss, nested, 1500 );

      BOOST_TEST_MESSAGE( "About to unpack." );
      fc::test::item unpacked;
      BOOST_CHECK_THROW( fc::raw::unpack( ss, unpacked ), fc::assert_exception );
   }

   // 150 levels, by default packing will fail
   {
      auto nested = create_nested_class_object( 150 );

      std::stringstream ss;

      BOOST_TEST_MESSAGE( "About to pack." );
      BOOST_CHECK_THROW( fc::raw::pack( ss, nested ), fc::assert_exception );
   }

   // 150 levels and allow packing, unpacking will fail
   {
      auto nested = create_nested_object( 150 );

      std::stringstream ss;

      BOOST_TEST_MESSAGE( "About to pack." );
      fc::raw::pack( ss, nested, 1500 );

      BOOST_TEST_MESSAGE( "About to unpack as class object." );
      fc::test::class_item unpacked;
      BOOST_CHECK_THROW( fc::raw::unpack( ss, unpacked ), fc::assert_exception );
   }

} FC_CAPTURE_LOG_AND_RETHROW ( (0) ) }

BOOST_AUTO_TEST_SUITE_END()
