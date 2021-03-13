#pragma once
#include <functional>
#include <memory>
#include <string>
#include <boost/any.hpp>
#include <fc/network/ip.hpp>
#include <fc/network/http/connection.hpp>
#include <fc/signals.hpp>

namespace fc { namespace http {
   namespace detail {
      class websocket_server_impl;
      class websocket_tls_server_impl;
      class websocket_client_impl;
      class websocket_tls_client_impl;
   } // namespace detail

   // TODO refactor code, move stuff used by server or client only to derived class(es),
   //      E.G. it seems get_request_header and on_http* are for server only.
   class websocket_connection
   {
      public:
         virtual ~websocket_connection(){}
         virtual void send_message( const std::string& message ) = 0;
         virtual void close( int64_t code, const std::string& reason  ){};
         void on_message( const std::string& message ) { _on_message(message); }
         fc::http::reply on_http( const std::string& message ) { return _on_http(message); }

         void on_message_handler( const std::function<void(const std::string&)>& h ) { _on_message = h; }
         void on_http_handler( const std::function<fc::http::reply(const std::string&)>& h ) { _on_http = h; }

         void        set_session_data( boost::any d ){ _session_data = std::move(d); }
         boost::any& get_session_data() { return _session_data; }

         virtual std::string get_request_header(const std::string& key) = 0;

         const std::string& get_remote_endpoint_string()const { return _remote_endpoint; }

         fc::signal<void()> closed;
      protected:
         std::string                               _remote_endpoint; // for logging
      private:
         boost::any                                _session_data;
         std::function<void(const std::string&)>   _on_message;
         std::function<fc::http::reply(const std::string&)> _on_http;
   };
   typedef std::shared_ptr<websocket_connection> websocket_connection_ptr;

   typedef std::function<void(const websocket_connection_ptr&)> on_connection_handler;

   // TODO websocket_tls_server and websocket_server have almost the same interface and implementation,
   //      better refactor to remove duplicate code and to avoid undesired or unnecessary differences
   class websocket_server
   {
      public:
         websocket_server(const std::string& forward_header_key = std::string());
         ~websocket_server();

         void on_connection( const on_connection_handler& handler);
         void listen( uint16_t port );
         void listen( const fc::ip::endpoint& ep );
         uint16_t get_listening_port();
         void start_accept();

         void stop_listening();
         void close();

      private:
         friend class detail::websocket_server_impl;
         std::unique_ptr<detail::websocket_server_impl> my;
   };


   // TODO websocket_tls_server and websocket_server have almost the same interface and implementation,
   //      better refactor to remove duplicate code and to avoid undesired or unnecessary differences
   class websocket_tls_server
   {
      public:
         websocket_tls_server( const std::string& server_pem,
                               const std::string& ssl_password,
                               const std::string& forward_header_key = std::string() );
         ~websocket_tls_server();

         void on_connection( const on_connection_handler& handler);
         void listen( uint16_t port );
         void listen( const fc::ip::endpoint& ep );
         uint16_t get_listening_port();
         void start_accept();

         void stop_listening();
         void close();

      private:
         friend class detail::websocket_tls_server_impl;
         std::unique_ptr<detail::websocket_tls_server_impl> my;
   };

   class websocket_client
   {
      public:
         websocket_client( const std::string& ca_filename = "_default" );
         ~websocket_client();

         websocket_connection_ptr connect( const std::string& uri );
         websocket_connection_ptr secure_connect( const std::string& uri );

         void close();
         void synchronous_close();
         void append_header(const std::string& key, const std::string& value);
      private:
         std::unique_ptr<detail::websocket_client_impl> my;
         std::unique_ptr<detail::websocket_tls_client_impl> smy;
         fc::http::headers _headers;
   };

} }
