#include <fc/api.hpp>
#include <fc/log/logger.hpp>
#include <fc/rpc/api_connection.hpp>
#include <fc/rpc/websocket_api.hpp>

class calculator
{
   public:
      int32_t add( int32_t a, int32_t b ); // not implemented
      int32_t sub( int32_t a, int32_t b ); // not implemented
      void    on_result( const std::function<void(int32_t)>& cb );
      void    on_result2( const std::function<void(int32_t)>& cb, int test );
};

FC_API( calculator, (add)(sub)(on_result)(on_result2) )


class login_api
{
   public:
      fc::api<calculator> get_calc()const
      {
         FC_ASSERT( calc );
         return *calc;
      }
      fc::optional<fc::api<calculator>> calc;
      std::set<std::string> test( const std::string&, const std::string& ) { return std::set<std::string>(); }
};
FC_API( login_api, (get_calc)(test) );


class optionals_api
{
public:
    std::string foo( const std::string& first, const fc::optional<std::string>& second,
                     const fc::optional<std::string>& third ) {
        return fc::json::to_string(fc::variants{first, {second, 2}, {third, 2}});
    }
};
FC_API( optionals_api, (foo) );

using namespace fc;

class some_calculator
{
   public:
      int32_t add( int32_t a, int32_t b ) { wlog("."); if( _cb ) _cb(a+b); return a+b; }
      int32_t sub( int32_t a, int32_t b ) {  wlog(".");if( _cb ) _cb(a-b); return a-b; }
      void    on_result( const std::function<void(int32_t)>& cb ) { wlog( "set callback" ); _cb = cb;  return ; }
      void    on_result2(  const std::function<void(int32_t)>& cb, int test ){}
      std::function<void(int32_t)> _cb;
};
class variant_calculator
{
   public:
      double add( fc::variant a, fc::variant b ) { return a.as_double()+b.as_double(); }
      double sub( fc::variant a, fc::variant b ) { return a.as_double()-b.as_double(); }
      void    on_result( const std::function<void(int32_t)>& cb ) { wlog("set callback"); _cb = cb; return ; }
      void    on_result2(  const std::function<void(int32_t)>& cb, int test ){}
      std::function<void(int32_t)> _cb;
};

using namespace fc::http;
using namespace fc::rpc;

#define MAX_DEPTH 10

int main( int argc, char** argv )
{
   try {
      fc::api<calculator> calc_api( std::make_shared<some_calculator>() );

      auto server = std::make_shared<fc::http::websocket_server>();
      server->on_connection([&]( const websocket_connection_ptr& c ){
               auto wsc = std::make_shared<websocket_api_connection>(c, MAX_DEPTH);
               auto login = std::make_shared<login_api>();
               login->calc = calc_api;
               wsc->register_api(fc::api<login_api>(login));
               c->set_session_data( wsc );
          });

      server->listen( 8090 );
      server->start_accept();

      try {
         auto client = std::make_shared<fc::http::websocket_client>();
         auto con  = client->connect( "ws://localhost:8090" );
         auto apic = std::make_shared<websocket_api_connection>(con, MAX_DEPTH);
         auto remote_login_api = apic->get_remote_api<login_api>();
         auto remote_calc = remote_login_api->get_calc();
         bool remote_triggered = false;
         remote_calc->on_result( [&remote_triggered]( uint32_t r ) { remote_triggered = true; } );
         FC_ASSERT(remote_calc->add( 4, 5 ) == 9);
         FC_ASSERT(remote_triggered);

         client.reset();
         fc::usleep(fc::milliseconds(100));
         server.reset();
      } FC_LOG_AND_RETHROW()
   } FC_LOG_AND_RETHROW()

   try {
      auto optionals = std::make_shared<optionals_api>();
      fc::api<optionals_api> oapi(optionals);
      FC_ASSERT(oapi->foo("a") == "[\"a\",null,null]");
      FC_ASSERT(oapi->foo("a", "b") == "[\"a\",\"b\",null]");
      FC_ASSERT(oapi->foo("a", "b", "c") == "[\"a\",\"b\",\"c\"]");
      FC_ASSERT(oapi->foo("a", {}, "c") == "[\"a\",null,\"c\"]");

      auto server = std::make_shared<fc::http::websocket_server>();
      server->on_connection([&]( const websocket_connection_ptr& c ){
               auto wsc = std::make_shared<websocket_api_connection>(*c, MAX_DEPTH);
               wsc->register_api(fc::api<optionals_api>(optionals));
               c->set_session_data( wsc );
          });

      server->listen( 8090 );
      server->start_accept();

      try {
         auto client = std::make_shared<fc::http::websocket_client>();
         auto con  = client->connect( "ws://localhost:8090" );
         auto apic = std::make_shared<websocket_api_connection>(*con, MAX_DEPTH);
         auto remote_optionals = apic->get_remote_api<optionals_api>();

         FC_ASSERT(remote_optionals->foo("a") == "[\"a\",null,null]");
         FC_ASSERT(remote_optionals->foo("a", "b") == "[\"a\",\"b\",null]");
         FC_ASSERT(remote_optionals->foo("a", "b", "c") == "[\"a\",\"b\",\"c\"]");
         FC_ASSERT(remote_optionals->foo("a", {}, "c") == "[\"a\",null,\"c\"]");

         client.reset();
         fc::usleep(fc::milliseconds(100));
         server.reset();
      } FC_LOG_AND_RETHROW()
   } FC_LOG_AND_RETHROW()

   return 0;
}
