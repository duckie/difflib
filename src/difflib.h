#ifndef __DIFFLIB__
#define __DIFFLIB__

#include <string>
#include <functional>
#include <utility>

namespace difflib {

// This trait checks if a given type is a standard collection of hashable types
// SFINAE is nice with C++11 and decltype
template <class T> class is_hashable_sequence {
  is_hashable_sequence() = delete;
  typedef char hashable;
  struct not_hashable { char t[2]; };  // Ensured to work on any platform
  template <typename C> static hashable matcher(decltype(&std::hash<typename C::value_type>::operator()));
  template <typename C> static not_hashable matcher(...);

 public:
  static bool const value = (sizeof(matcher<T>(nullptr)) == sizeof(hashable));
};

template <class T = std::string> class SequenceMatcher {
  static_assert(is_hashable_sequence<T>::value, "The matched sequences must be of hashable elements.");
 public:
  SequenceMatcher(T const& a, T const& b) : a_(a), b_(b) {}
  SequenceMatcher(SequenceMatcher<T> const&) = delete;
  SequenceMatcher& operator= (SequenceMatcher<T> const&) = delete;
  SequenceMatcher(SequenceMatcher<T>&&) = default;
  SequenceMatcher& operator= (SequenceMatcher<T>&&) = default;

 private:
  T a_;
  T b_;
};

template <class T> auto MakeSequenceMatcher(T const& a, T const& b) -> SequenceMatcher<T> { 
  return SequenceMatcher<T>(a,b);
}

}
#endif  // __DIFFLIB__
