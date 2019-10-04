#pragma once
#include <fc/fwd.hpp>
#include <fc/string.hpp>

namespace fc
{

class md5 
{
  public:
    md5();
    explicit md5( const string& hex_str );

    string str()const;
    operator string()const;

    char*    data()const;
    size_t data_size()const { return 128 / 8; }

    static md5 hash( const char* d, uint32_t dlen );
    static md5 hash( const string& );

    template<typename T>
    static md5 hash( const T& t ) 
    { 
      md5::encoder e; 
      e << t; 
      return e.result(); 
    } 

    class encoder 
    {
      public:
        encoder();
        ~encoder();

        void write( const char* d, uint32_t dlen );
        void put( char c ) { write( &c, 1 ); }
        void reset();
        md5 result();

      private:
        struct      impl;
        fc::fwd<impl,216> my;
    };

    template<typename T>
    inline friend T& operator<<( T& ds, const md5& ep ) {
      ds.write( ep.data(), sizeof(ep) );
      return ds;
    }

    template<typename T>
    inline friend T& operator>>( T& ds, md5& ep ) {
      ds.read( ep.data(), sizeof(ep) );
      return ds;
    }
    friend md5 operator << ( const md5& h1, uint32_t i       );
    friend bool   operator == ( const md5& h1, const md5& h2 );
    friend bool   operator != ( const md5& h1, const md5& h2 );
    friend md5 operator ^  ( const md5& h1, const md5& h2 );
    friend bool   operator >= ( const md5& h1, const md5& h2 );
    friend bool   operator >  ( const md5& h1, const md5& h2 ); 
    friend bool   operator <  ( const md5& h1, const md5& h2 ); 
                             
    uint64_t _hash[2]; 
};

  class variant;
  void to_variant( const md5& bi, variant& v, uint32_t max_depth = 1 );
  void from_variant( const variant& v, md5& bi, uint32_t max_depth = 1 );

} // fc

#include <fc/reflect/reflect.hpp>
FC_REFLECT_TYPENAME( fc::md5 )
