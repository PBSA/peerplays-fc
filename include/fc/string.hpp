#pragma once
#include <fc/fwd.hpp>
#include <fc/optional.hpp>
#include <string>

namespace fc
{
    using std::string;

  int64_t  to_int64( const std::string& );
  uint64_t to_uint64( const std::string& );
  double   to_double( const std::string& );
  std::string to_string( double );
  std::string to_string( uint64_t );
  std::string to_string( int64_t );
  std::string to_string( uint16_t );
  std::string to_pretty_string( int64_t );
  inline std::string to_string( int32_t v ) { return to_string( int64_t(v) ); }
  inline std::string to_string( uint32_t v ){ return to_string( uint64_t(v) ); }
#if defined(__APPLE__) or defined(__OpenBSD__)
  inline std::string to_string( size_t s) { return to_string(uint64_t(s)); }
#endif

  typedef fc::optional<std::string> ostring;
  class variant_object;
  std::string format_string( const std::string&, const variant_object&, uint32_t max_object_depth = 200 );
  std::string trim( const std::string& );
  std::string to_lower( const std::string& );
  string trim_and_normalize_spaces( const string& s );

  uint64_t parse_size( const string& s );
}
