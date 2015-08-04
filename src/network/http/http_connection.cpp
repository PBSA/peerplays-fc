#include <fc/network/http/connection.hpp>
#include <fc/network/tcp_socket.hpp>
#include <fc/io/sstream.hpp>
#include <fc/io/iostream.hpp>
#include <fc/exception/exception.hpp>
#include <fc/network/ip.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/stdio.hpp>
#include <fc/network/url.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

class fc::http::connection::impl 
{
  public:
   fc::tcp_socket sock;
   fc::ip::endpoint ep;
   impl() {}

   size_t read_until(std::shared_ptr<char> buffer, size_t buffer_length, char c = '\n') 
   {
     size_t offset = 0;
     while (offset < buffer_length && 
         sock.readsome(buffer, 1, offset) == 1) 
     {
       if (buffer.get()[offset] == c)
       {
         buffer.get()[offset] = 0;
         return offset;
       }
       ++offset;
     }
     return offset;
   }

   fc::http::reply parse_reply() 
   {
      fc::http::reply rep;
      fc::oexception parsing_exception;
      try 
      {
        const size_t buffer_length = 1024 * 8;
        std::shared_ptr<char> line(new char[buffer_length], [](char* p){ delete[] p; });
        read_until(line, buffer_length, ' '); // HTTP/1.1
        size_t bytes_read = read_until(line, buffer_length, ' '); // CODE
        rep.status = static_cast<int>(to_int64(fc::string(line.get(), bytes_read)));
        read_until(line, buffer_length, '\n'); // DESCRIPTION
        
        fc::optional<size_t> content_length;
        bool is_chunked = false;
        while( (bytes_read = read_until(line, buffer_length, '\n')) > 1 ) 
        {
          fc::http::header h;
          std::string line_string(line.get(), bytes_read);
          size_t colon_pos = line_string.find(": ");
          if (colon_pos != std::string::npos)
          {
            h.key = line_string.substr(0, colon_pos);
            size_t value_start_pos = colon_pos + 2;
            size_t carriage_return_pos = line_string.find('\r', value_start_pos);
            if (carriage_return_pos != std::string::npos)
              h.val = line_string.substr(value_start_pos, carriage_return_pos - value_start_pos);
            else
              h.val = line_string.substr(value_start_pos);
          }
          rep.headers.push_back(h);
          if( boost::iequals(h.key, "Content-Length") )
            content_length = static_cast<size_t>(to_uint64( fc::string(h.val) ));
          if( boost::iequals(h.key, "Transfer-Encoding") && 
              boost::iequals(fc::string(h.val), "chunked") )
            is_chunked = true;
        }

        if (is_chunked)
        {
          do
          {
            // Chunked means we get a hexadecimal number of bytes on a line, followed by the content
            bytes_read = read_until(line, buffer_length, '\n'); //read chunk length 
            std::string line_string(line.get(), bytes_read);
            if (line_string.size() > 0 && line_string[line_string.size() - 1] == '\r')
              line_string.erase(line_string.size() - 1);
            unsigned length;
            if (sscanf(line_string.c_str(), "%x", &length) != 1)
              FC_THROW("Invalid content length: ${length}", ("length", line_string));
            content_length = length;
            if (*content_length)
            {
              std::shared_ptr<char> temp_data(new char[*content_length], [](char* p){ delete[] p; });
              sock.read(temp_data, *content_length, 0);
              boost::push_back(rep.body, std::make_pair(temp_data.get(), temp_data.get() + *content_length));
              read_until(line, buffer_length, '\n'); //discard cr/lf after each chunk 
            }
          }
          while (*content_length != 0);
        }

        if (content_length)
        {
          if (*content_length)
          {
            std::shared_ptr<char> temp_data(new char[*content_length], [](char* p){ delete[] p; });
            sock.read(temp_data, *content_length, 0);
            boost::push_back(rep.body, std::make_pair(temp_data.get(), temp_data.get() + *content_length));
          }
        }
        else //just read until closed if no content length or chunking
        {
          while (true)
          {
            try
            {
              sock.read(line, 1, 0);
              rep.body.push_back(line.get()[0]);
            }
            catch (const fc::canceled_exception&)
            {
              throw;
            }
            catch (const fc::eof_exception&)
            {
              break;
            }
          }
        }

        return rep;
      } 
      catch (const fc::canceled_exception&)
      {
        throw;
      }
      catch (const fc::exception& e) 
      {
        parsing_exception = e;
      } 
      assert(parsing_exception); // the only way we get here is if the last catch falls through
      elog("${exception}", ("exception", parsing_exception->to_detail_string()));
      sock.close();
      rep.status = http::reply::InternalServerError;
      return rep;
   }
};



namespace fc { namespace http {

connection::connection() :
  my( new connection::impl() )
{}

connection::~connection(){}

// used for clients
void connection::connect_to( const fc::ip::endpoint& ep ) 
{
  my->sock.close();
  my->sock.connect_to( my->ep = ep );
}

http::reply connection::request( const fc::string& method, 
                                 const fc::string& url, 
                                 const fc::string& body, 
                                 const headers& he,
                                 const fc::string& content_type ) 
{
  fc::url parsed_url(url);
  if( !my->sock.is_open() ) 
  {
    wlog( "Re-open socket!" );
    my->sock.connect_to( my->ep );
  }
  try 
  {
    fc::stringstream req;
    req << method << " " << parsed_url.path()->generic_string() << parsed_url.args_as_string() << " HTTP/1.1\r\n";
    req << "Host: " << *parsed_url.host() << "\r\n";
    req << "Content-Type: " << content_type << "\r\n";
    for( auto i = he.begin(); i != he.end(); ++i )
      req << i->key << ": " << i->val << "\r\n";
    if( body.size() ) 
      req << "Content-Length: "<< body.size() << "\r\n";
    req << "\r\n"; 

    {
      fc::string head = req.str();
      std::shared_ptr<char> write_buffer(new char[head.size()], [](char* p){ delete[] p; });
      std::copy(head.begin(), head.end(), write_buffer.get());
      my->sock.write(write_buffer, head.size(), 0);
      //elog("Sending header ${head}", ("head", head));
      //  fc::cerr.write( head.c_str() );
    }

    if( body.size() )  
    {
      std::shared_ptr<char> write_buffer(new char[body.size()], [](char* p){ delete[] p; });
      std::copy(body.begin(), body.end(), write_buffer.get());
      my->sock.write(write_buffer, body.size(), 0);
      //elog("Sending body ${body}", ("body", body));
      //fc::cerr.write( body.c_str() );
    }
    // fc::cerr.flush();

    return my->parse_reply();
  } 
  catch (const fc::canceled_exception&)
  {
    throw;
  }
  catch (...) 
  {
    // fall through
  }
  // the only way we get here is if we encountered catch(...)
  my->sock.close();
  FC_THROW_EXCEPTION( exception, "Error Sending HTTP Request" ); // TODO: provide more info
  // return http::reply( http::reply::InternalServerError ); // TODO: replace with connection error
}

// used for servers
fc::tcp_socket& connection::get_socket()const {
  return my->sock;
}

http::request connection::read_request()const {
  http::request req;
  const size_t buffer_length = 1024 * 8;
  std::shared_ptr<char> line(new char[buffer_length], [](char* p){ delete[] p; });
  size_t bytes_read = my->read_until(line, buffer_length, ' '); // METHOD
  req.method = std::string(line.get(), bytes_read);
  bytes_read = my->read_until(line, buffer_length, ' '); // PATH
  req.path = std::string(line.get(), bytes_read);
  bytes_read = my->read_until(line, buffer_length, '\n'); // HTTP/1.0
  
  while( (bytes_read = my->read_until(line, buffer_length, '\n')) > 1 ) 
  {
    fc::http::header h;
    std::string line_string(line.get(), bytes_read);
    size_t colon_pos = line_string.find(": ");
    if (colon_pos != std::string::npos)
    {
      h.key = line_string.substr(0, colon_pos);
      size_t value_start_pos = colon_pos + 2;
      size_t carriage_return_pos = line_string.find('\r', value_start_pos);
      if (carriage_return_pos != std::string::npos)
        h.val = line_string.substr(value_start_pos, carriage_return_pos - value_start_pos);
      else
        h.val = line_string.substr(value_start_pos);
    }
    req.headers.push_back(h);
    if( boost::iequals(h.key, "Content-Length")) 
    {
      size_t content_length = static_cast<size_t>(to_uint64( fc::string(h.val) ) );
      FC_ASSERT(content_length < 1024*1024);
      req.body.resize( static_cast<size_t>(to_uint64( fc::string(h.val) ) ));
    }
    if( boost::iequals(h.key, "Host") )
      req.domain = h.val;
  }
  // TODO: some common servers won't give a Content-Length, they'll use 
  // Transfer-Encoding: chunked.  handle that here.

  if( req.body.size() )
  {
    std::shared_ptr<char> body_buffer(new char[req.body.size()], [](char* p){ delete[] p; });
    my->sock.read(body_buffer, req.body.size(), 0);
    std::copy(body_buffer.get(), body_buffer.get() + req.body.size(), req.body.data());
  }

  return req;
}

fc::string request::get_header( const fc::string& key )const 
{
  for( auto itr = headers.begin(); itr != headers.end(); ++itr )
    if( boost::iequals(itr->key, key) )
      return itr->val;
  return fc::string();
}

std::vector<header> parse_urlencoded_params( const fc::string& f ) 
{
  int num_args = 0;
  for( size_t i = 0; i < f.size(); ++i )
    if( f[i] == '=' ) 
      ++num_args;
  
  std::vector<header> h(num_args);
  int arg = 0;
  for( size_t i = 0; i < f.size(); ++i )
  {
    while( f[i] != '=' && i < f.size() ) 
    {
      if( f[i] == '%' ) 
      {
        h[arg].key += char((fc::from_hex(f[i+1]) << 4) | fc::from_hex(f[i+2]));
        i += 3;
      }
      else 
      {
        h[arg].key += f[i];
        ++i;
      }
    }
    ++i;

    while( i < f.size() && f[i] != '&' ) 
    {
      if( f[i] == '%' ) 
      { 
        h[arg].val += char((fc::from_hex(f[i+1]) << 4) | fc::from_hex(f[i+2]));
        i += 3;
      } 
      else
      {
        h[arg].val += f[i] == '+' ? ' ' : f[i];
        ++i;
      }
    }
    ++arg;
  }
  return h;
}

} } // fc::http
