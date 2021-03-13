#pragma once

#include <array>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <fc/optional.hpp>
#include <fc/string.hpp>
#include <fc/container/flat_fwd.hpp>

namespace fc {
  template<typename...> class static_variant;
  class value;
  class exception;
  namespace ip { class address; }

  template<typename... T> struct get_typename;
#if defined(__APPLE__) or defined(__OpenBSD__)
  template<> struct get_typename<size_t>   { static const char* name()  { return "size_t";   } };
#endif
  template<> struct get_typename<int32_t>  { static const char* name()  { return "int32_t";  } };
  template<> struct get_typename<int64_t>  { static const char* name()  { return "int64_t";  } };
  template<> struct get_typename<int16_t>  { static const char* name()  { return "int16_t";  } };
  template<> struct get_typename<int8_t>   { static const char* name()  { return "int8_t";   } };
  template<> struct get_typename<uint32_t> { static const char* name()  { return "uint32_t"; } };
  template<> struct get_typename<uint64_t> { static const char* name()  { return "uint64_t"; } };
  template<> struct get_typename<uint16_t> { static const char* name()  { return "uint16_t"; } };
  template<> struct get_typename<uint8_t>  { static const char* name()  { return "uint8_t";  } };
  template<> struct get_typename<double>   { static const char* name()  { return "double";   } };
  template<> struct get_typename<float>    { static const char* name()  { return "float";    } };
  template<> struct get_typename<bool>     { static const char* name()  { return "bool";     } };
  template<> struct get_typename<char>     { static const char* name()  { return "char";     } };
  template<> struct get_typename<void>     { static const char* name()  { return "char";     } };
  template<> struct get_typename<value>    { static const char* name()   { return "value";   } };
  template<> struct get_typename<std::string> { static const char* name()  { return "string";   } };
  template<> struct get_typename<fc::exception>   { static const char* name()   { return "fc::exception";   } };
  template<> struct get_typename<std::vector<char>>   { static const char* name()   { return "std::vector<char>";   } };
  template<typename T> struct get_typename<std::vector<T>>   
  { 
     static const char* name()  { 
         static std::string n = std::string("std::vector<") + get_typename<T>::name() + ">"; 
         return n.c_str();  
     } 
  };
  template<typename T> struct get_typename<flat_set<T>>
  {
     static const char* name()  {
         static std::string n = std::string("flat_set<") + get_typename<T>::name() + ">";
         return n.c_str();
     }
  };
  template<typename... Ts>
  struct get_typename<flat_set<static_variant<Ts...>, typename static_variant<Ts...>::type_lt>>
  { 
     static const char* name()  { 
         using TN = get_typename<static_variant<Ts...>>;
         static std::string n = std::string("flat_set<") + TN::name() + ", " + TN::name() + "::type_lt>";
         return n.c_str();  
     } 
  };
  template<typename T, typename U> struct get_typename<flat_map<T, U>>
  {
     static const char* name()  {
         static std::string n = std::string("flat_map<") + get_typename<T>::name() + ", " + get_typename<U>::name() + ">";
         return n.c_str();
     }
  };
  template<typename T> struct get_typename< std::deque<T> >
  {
     static const char* name()
     {
        static std::string n = std::string("std::deque<") + get_typename<T>::name() + ">"; 
        return n.c_str();  
     }
  };
  template<typename T> struct get_typename<optional<T>>   
  { 
     static const char* name()  { 
         static std::string n = std::string("optional<") + get_typename<T>::name() + ">"; 
         return n.c_str();  
     } 
  };
  template<typename K,typename V> struct get_typename<std::map<K,V>>   
  { 
     static const char* name()  { 
         static std::string n = std::string("std::map<") + get_typename<K>::name() + ","+get_typename<V>::name()+">"; 
         return n.c_str();  
     } 
  };
  template<typename E> struct get_typename< std::set<E> >
  {
     static const char* name()
     {
        static std::string n = std::string("std::set<") + std::string(get_typename<E>::name()) + std::string(">");
        return n.c_str();
     }
  };
 template<typename A, typename B> struct get_typename< std::pair<A,B> >
  {
      static const char* name()
      {
         static std::string n = std::string("std::pair<") + get_typename<A>::name() + "," + get_typename<B>::name() + ">";
         return n.c_str();
      }
  }; 
  template<typename T,size_t N> struct get_typename< std::array<T,N> >  
  { 
     static const char* name()  
     { 
        static std::string _name = std::string("std::array<") + std::string(fc::get_typename<T>::name())
                                   + "," + fc::to_string(N) + ">";
        return _name.c_str();
     } 
  }; 
  template<typename T> struct get_typename< const T* >
  {
      static const char* name()
      {
         static std::string n = std::string("const ") + get_typename<T>::name() + "*";
         return n.c_str();
      }
  };
  template<typename T> struct get_typename< T* >
  {
      static const char* name()
      {
         static std::string n = std::string(get_typename<T>::name()) + "*";
         return n.c_str();
      }
  };

  struct signed_int;
  struct unsigned_int;
  class variant_object;
  template<> struct get_typename<signed_int>   { static const char* name()   { return "signed_int";   } };
  template<> struct get_typename<unsigned_int>   { static const char* name()   { return "unsigned_int";   } };
  template<> struct get_typename<variant_object>   { static const char* name()   { return "fc::variant_object";   } };

}
