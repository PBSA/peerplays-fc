#include <fc/crypto/hex.hpp>
#include <fc/fwd_impl.hpp>
#include <openssl/md5.h>
#include <string.h>
#include <fc/crypto/md5.hpp>
#include <fc/variant.hpp>
  
namespace fc {

    md5::md5() { memset( _hash, 0, sizeof(_hash) ); }
    md5::md5( const string& hex_str ) {
      fc::from_hex( hex_str, (char*)_hash, sizeof(_hash) );  
    }

    string md5::str()const {
      return fc::to_hex( (char*)_hash, sizeof(_hash) );
    }
    md5::operator string()const { return  str(); }

    char* md5::data()const { return (char*)&_hash[0]; }


    struct md5::encoder::impl {
       MD5_CTX ctx;
    };

    md5::encoder::~encoder() {}
    md5::encoder::encoder() {
      reset();
    }

    md5 md5::hash( const char* d, uint32_t dlen ) {
      encoder e;
      e.write(d,dlen);
      return e.result();
    }
    md5 md5::hash( const string& s ) {
      return hash( s.c_str(), s.size() );
    }

    void md5::encoder::write( const char* d, uint32_t dlen ) {
      MD5_Update( &my->ctx, d, dlen); 
    }
    md5 md5::encoder::result() {
      md5 h;
      MD5_Final((uint8_t*)h.data(), &my->ctx );
      return h;
    }
    void md5::encoder::reset() {
      MD5_Init( &my->ctx);  
    }

    md5 operator << ( const md5& h1, uint32_t i ) {
      md5 result;
      uint8_t* r = (uint8_t*)result._hash;
      uint8_t* s = (uint8_t*)h1._hash;
      for( uint32_t p = 0; p < sizeof(h1._hash)-1; ++p )
          r[p] = s[p] << i | (s[p+1]>>(8-i));
      r[63] = s[63] << i;
      return result;
    }
    md5 operator ^ ( const md5& h1, const md5& h2 ) {
      md5 result;
      result._hash[0] = h1._hash[0] ^ h2._hash[0];
      result._hash[1] = h1._hash[1] ^ h2._hash[1];
      return result;
    }
    bool operator >= ( const md5& h1, const md5& h2 ) {
      return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) >= 0;
    }
    bool operator > ( const md5& h1, const md5& h2 ) {
      return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) > 0;
    }
    bool operator < ( const md5& h1, const md5& h2 ) {
      return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) < 0;
    }
    bool operator != ( const md5& h1, const md5& h2 ) {
      return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) != 0;
    }
    bool operator == ( const md5& h1, const md5& h2 ) {
      return memcmp( h1._hash, h2._hash, sizeof(h1._hash) ) == 0;
    }
  
  void to_variant( const md5& bi, variant& v, uint32_t max_depth )
  {
     v = fc::variant( std::vector<char>( (const char*)&bi, ((const char*)&bi) + sizeof(bi) ), max_depth );
  }
  void from_variant( const variant& v, md5& bi, uint32_t max_depth )
  {
    std::vector<char> ve = v.as< std::vector<char> >(max_depth);
    if( ve.size() )
        memcpy(&bi, ve.data(), fc::min<size_t>(ve.size(),sizeof(bi)) );
    else
        for (size_t i = 0; i < bi.data_size(); i++) {
            bi.data()[i] = 0;
        }
  }
}
