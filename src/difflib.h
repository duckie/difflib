#ifndef __DIFFLIB__
#define __DIFFLIB__

#include <string>
#include <functional>
#include <utility>
#include <iterator>
#include <type_traits>
#include <iostream>
#include <unordered_map>
#include <list>
#include <algorithm>

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

template <class T> class is_standard_iterable {
  is_standard_iterable () = delete;
  typedef char iterable;
  struct not_iterable { char t[2]; };  // Ensured to work on any platform
  template <typename C> static iterable matcher(typename C::const_iterator*);
  template <typename C> static not_iterable matcher(...);

 public:
  static bool const value = (sizeof(matcher<T>(nullptr)) == sizeof(iterable));
};

// "H" for hashable
template <class H> bool NoJunk(H const&)  { return false; }

template <class T = std::string> class SequenceMatcher {
  static_assert(is_standard_iterable<T>::value, "The matched objects must be iterable.");
  static_assert(is_hashable_sequence<T>::value, "The matched sequences must be of hashable elements.");

 public:
  using value_type = T;
  using hashable_type = typename T::value_type;
  using junk_function_type = std::function<bool(hashable_type const&)>;

  SequenceMatcher(T const& a, T const& b, junk_function_type is_junk = NoJunk<hashable_type>): 
  a_(a), b_(b), is_junk_(is_junk) {
    set_seq(a,b);
  }

  SequenceMatcher(SequenceMatcher<T> const&) = delete;
  SequenceMatcher& operator= (SequenceMatcher<T> const&) = delete;
  SequenceMatcher(SequenceMatcher<T>&&) = default;
  SequenceMatcher& operator= (SequenceMatcher<T>&&) = default;

  void set_seq(T const& a, T const& b) {
    set_seq1(a);
    set_seq2(b);
  }

  void set_seq1(T const& a) { 
    a_ = a;
  }

  void set_seq2(T const& b) {
    b_ = b;
    size_t index=0;
    for(hashable_type const& elem : b_) b2j_[elem].push_back(index++);
    for(auto current_it = b2j_.begin(); current_it != b2j_.end();) {
      if(is_junk_(current_it->first)) current_it = b2j_.erase(current_it);
      else ++current_it;
    }
  }


 private:
  using b2j_t = std::unordered_map<hashable_type, std::list<size_t>>;

  T a_;
  T b_;
  b2j_t b2j_; 
  junk_function_type is_junk_;
};

template <class T> auto MakeSequenceMatcher(T const& a, T const& b) -> SequenceMatcher<T> { 
  return SequenceMatcher<T>(a,b);
}

}
#endif  // __DIFFLIB__
