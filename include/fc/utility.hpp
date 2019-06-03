#pragma once
#include <stdint.h>
#include <cstdlib>

namespace fc {
  using std::size_t;

  struct true_type  { enum _value { value = 1 }; };
  struct false_type { enum _value { value = 0 }; };

  namespace detail {
    template<typename T> fc::true_type is_class_helper(void(T::*)());
    template<typename T> fc::false_type is_class_helper(...);
  }

  template<typename T>
  struct is_class { typedef decltype(detail::is_class_helper<T>(0)) type; enum value_enum { value = type::value }; };
}
