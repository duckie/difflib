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
#include <unordered_set>
#include <tuple>

namespace difflib {

size_t const AUTOJUNK_MIN_SIZE = 200; 

// This trait checks if a given type is a standard collection of hashable types
// SFINAE ftw
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

template <typename C> void* sub_matcher(typename C::value_type const& (C::*)(size_t) const);

// TODO: Not a real SFINAE, because of the ambiguity between
// value_type const& operator[](size_t) const;
// and value_type& operator[](size_t);
// Not really important
template <class T> class has_bracket_operator {
  has_bracket_operator () = delete;
  typedef char has_op;
  struct hasnt_op { char t[2]; };  // Ensured to work on any platform
  template <typename C> static has_op matcher(decltype(sub_matcher<T>(&T::at)));
  template <typename C> static hasnt_op matcher(...); 
 public:
  static bool const value = (sizeof(matcher<T>(nullptr)) == sizeof(has_op));
};

// "H" for hashable
template <class H> bool NoJunk(H const&)  { return false; }

template <class T = std::string> class SequenceMatcher {
  static_assert(is_standard_iterable<T>::value, "The matched objects must be iterable.");
  static_assert(is_hashable_sequence<T>::value, "The matched sequences must be of hashable elements.");
  static_assert(has_bracket_operator<T>::value, "The matched sequences must implement operator[].");

 public:
  using value_type = T;
  using hashable_type = typename T::value_type;
  using junk_function_type = std::function<bool(hashable_type const&)>;
  using match_t = std::tuple<size_t, size_t, size_t>;

  SequenceMatcher(T const& a, T const& b, junk_function_type is_junk = NoJunk<hashable_type>, bool auto_junk = true): a_(a), b_(b), is_junk_(is_junk), auto_junk_(auto_junk) {
    chain_b();
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
    chain_b(); 
  }
  
  match_t find_longest_match(size_t a_low, size_t a_high, size_t b_low, size_t b_high) {
    size_t best_i = a_low;
    size_t best_j = b_low;
    size_t best_size = 0;

    // Find longest junk free match
    {
      std::unordered_map<size_t, size_t> j2len;
      for(size_t i = a_low; i < a_high; ++i) {
        std::unordered_map<size_t, size_t> new_j2len;
        for(size_t j : b2j_[a_[i]]) {
          if (j < b_low) continue;
          if (j >= b_high) break;
          size_t k = new_j2len[i] = j2len[j-1] + 1;
          if (k > best_size) {
            best_i = i-k+1;
            best_j = j-k+1;
            best_size = k;
          }
        }
        j2len = std::move(new_j2len);
      }
    }
    
    // Extend the best by non-junk elements on each end
    auto low_bound_test = [&best_i, &best_j, a_low, b_low, &best_size, this] (bool isjunk) {
      while (
        best_i > a_low  
        && best_j > b_low 
        && this->a_[best_i-1] == this->b_[best_j-1] 
        && isjunk == b2j_.count(b_[best_j-1])
      ) {
        --best_i; --best_j; ++best_size;
      }
    };

    auto high_bound_test = [best_i, best_j, a_high, b_high, &best_size, this] (bool isjunk) {
      while (
        (best_i+best_size) < a_high  
        && (best_j+best_size) < b_high 
        && this->a_[best_i+best_size] == this->b_[best_j+best_size]
        && isjunk == b2j_.count(b_[best_j + best_size])
      ) {
        ++best_size;
      }
    };

    low_bound_test(false);
    high_bound_test(false);

    // Adds matching junks at each end
    low_bound_test(true);
    high_bound_test(true);

    return std::make_tuple(best_i, best_j, best_size);
  }

 private:
  using b2j_t = std::unordered_map<hashable_type, std::list<size_t>>;
  using junk_set_t = std::unordered_set<hashable_type>;

  void chain_b() {
    size_t index=0;
    
    // Counting occurences
    for(hashable_type const& elem : b_) b2j_[elem].push_back(index++);
    
    // Purge junk elements
    junk_set_.clear();
    for(auto it = b2j_.begin(); it != b2j_.end();) {
      if(is_junk_(it->first)) {
        junk_set_.insert(it->first);
        it = b2j_.erase(it);
      }
      else {
        ++it;
      }
    }
    
    // Purge popular elements that are not junk
    popular_set_.clear();
    if (auto_junk_ && AUTOJUNK_MIN_SIZE < b_.size()) {
      size_t ntest = b_.size()/100 + 1;
      for(auto it = b2j_.begin(); it != b2j_.end();) {
        if (ntest < it->second.size()) {
          popular_set_.insert(it->first);
          it = b2j_.erase(it);
        }
        else {
          ++it;
        }
      }
    }
  }

  T a_;
  T b_;
  junk_function_type is_junk_;
  bool auto_junk_;
  b2j_t b2j_;
  junk_set_t junk_set_;
  junk_set_t popular_set_;

};

template <class T> auto MakeSequenceMatcher(
  T const& a
  , T const& b
  , typename SequenceMatcher<T>::junk_function_type is_junk = NoJunk<typename SequenceMatcher<T>::hashable_type>
)
-> SequenceMatcher<T> 
{ 
  return SequenceMatcher<T>(a, b, is_junk);
}

}
#endif  // __DIFFLIB__
