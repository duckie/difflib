#ifndef __DIFFLIB__
#define __DIFFLIB__

#include <string>
#include <functional>
#include <utility>
#include <iterator>
#include <type_traits>
#include <iostream>
#include <unordered_map>
#include <map>
#include <list>
#include <algorithm>
#include <unordered_set>
#include <set>
#include <tuple>
#include <algorithm>
#include <memory>
#include <vector>

namespace difflib {

using std::vector;
using std::list;
using std::tuple;
using std::make_tuple;
using std::tie;

// Exposed types
using match_t = tuple<size_t, size_t, size_t>;
using match_list_t = std::vector<match_t>;  // A vector to speed up copying

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

  SequenceMatcher(T const& a, T const& b, junk_function_type is_junk = NoJunk<hashable_type>, bool auto_junk = true): a_(a), b_(b), is_junk_(is_junk), auto_junk_(auto_junk) {
    j2len_.resize(b.size()+1);
    chain_b();
  }

  SequenceMatcher(SequenceMatcher<T> const&) = delete;
  SequenceMatcher& operator= (SequenceMatcher<T> const&) = delete;
  SequenceMatcher(SequenceMatcher<T>&&) = default;
  SequenceMatcher& operator= (SequenceMatcher<T>&&) = default;

  std::size_t auto_junk_minsize() const { return auto_junk_minsize_; }
  void set_auto_junk_minsize(std::size_t value) { auto_junk_minsize_ = value; }

  void set_seq(T const& a, T const& b) {
    set_seq1(a);
    set_seq2(b);
  }

  void set_seq1(T const& a) { 
    a_ = a;
    matching_blocks_ = nullptr;
  }

  void set_seq2(T const& b) {
    b_ = b;
    j2len_.resize(b.size()+1);
    chain_b();
    matching_blocks_ = nullptr;
  }
  
  double ratio() {
    size_t sum = 0;
    size_t length = a_.size()+b_.size();
    if(length==0) return 1.0;
    for(match_t  m : get_matching_blocks())
        sum+=std::get<2>(m);
    return 2.*sum/length;
  } 
  
  match_t find_longest_match(size_t a_low, size_t a_high, size_t b_low, size_t b_high) {
    using std::begin;
    using std::end;


    size_t best_i = a_low;
    size_t best_j = b_low;
    size_t best_size = 0;
    
    // Find longest junk free match
    {
      j2_values_to_erase_.clear();
      for(size_t i = a_low; i < a_high; ++i) {
        j2_values_to_affect_.clear();

        for(size_t j : b2j_[a_[i]]) {
          if (j < b_low) continue;
          if (j >= b_high) break;
          size_t k = j2len_[j] + 1;
          j2_values_to_affect_.emplace_back(j+1,k);
          if (k > best_size) {
            best_i = i-k+1;
            best_j = j-k+1;
            best_size = k;
          }
        }
        
        for(auto const& elem : j2_values_to_erase_) {
          j2len_[elem.first] = 0;
        }
        for(auto const& elem : j2_values_to_affect_) {
          j2len_[elem.first] = elem.second;
        }
        std::swap(j2_values_to_erase_, j2_values_to_affect_);
      }
      for(auto const& elem : j2_values_to_erase_) {
        j2len_[elem.first] = 0;
      }
    }
   
    // Utility lambdas for factoring 
    auto low_bound_expand = [&best_i, &best_j, a_low, b_low, &best_size, this] (bool isjunk) {
      while (
        best_i > a_low  
        && best_j > b_low 
        && this->a_[best_i-1] == this->b_[best_j-1] 
        && isjunk == b2j_.count(b_[best_j-1])
      ) {
        --best_i; --best_j; ++best_size;
      }
    };

    // References on best_i best_k are needed here even if modified inside the code
    // because modified betweent the calls
    auto high_bound_expand = [&best_i, &best_j, a_high, b_high, &best_size, this] (bool isjunk) {
      while (
        (best_i+best_size) < a_high  
        && (best_j+best_size) < b_high 
        && this->a_[best_i+best_size] == this->b_[best_j+best_size]
        && isjunk == b2j_.count(b_[best_j + best_size])
      ) {
        ++best_size;
      }
    };

    // Extend the best by non-junk elements on each end
    low_bound_expand(false);
    high_bound_expand(false);

    // Adds matching junks at each end
    low_bound_expand(true);
    high_bound_expand(true);

    return make_tuple(best_i, best_j, best_size);
  }

  match_list_t get_matching_blocks() {
    // The following are tuple extracting aliases
    using std::get;

    if (matching_blocks_)
      return *matching_blocks_;
    
    vector<tuple<size_t, size_t, size_t, size_t>> queue;
    vector<match_t> matching_blocks_pass1;

    std::size_t queue_head = 0;
    queue.reserve(std::min(a_.size(), b_.size()));
    queue.emplace_back(0, a_.size(), 0, b_.size());

    while(queue_head < queue.size()) {
      size_t a_low, a_high, b_low, b_high;
      tie(a_low, a_high, b_low, b_high) = queue[queue_head++];
      match_t m = find_longest_match(a_low, a_high, b_low, b_high);
      if (get<2>(m)) {
        if (a_low < get<0>(m) && b_low < get<1>(m)) {
          queue.emplace_back(a_low, get<0>(m), b_low, get<1>(m));
        }
        if ((get<0>(m)+get<2>(m)) < a_high && (get<1>(m)+get<2>(m)) < b_high) {
          queue.emplace_back(get<0>(m)+get<2>(m), a_high, get<1>(m)+get<2>(m), b_high);
        }
        matching_blocks_pass1.push_back(std::move(m));
      }
    }
    std::sort(std::begin(matching_blocks_pass1), end(matching_blocks_pass1));
    
    matching_blocks_.reset(new match_list_t);
    matching_blocks_->reserve(matching_blocks_pass1.size());
    
    size_t i1, j1, k1;
    i1 = j1 = k1 = 0;

    for(match_t const& m : matching_blocks_pass1) {
      if (i1 + k1 == get<0>(m) && j1 + k1 == get<1>(m)) {
        k1 += get<2>(m);
      }
      else {
        if (k1) matching_blocks_->emplace_back(i1, j1, k1);
        std::tie(i1, j1, k1) = m;
      }
    }
    if (k1) matching_blocks_->emplace_back(i1, j1, k1);
    matching_blocks_->emplace_back(a_.size(), b_.size(), 0);

    return *matching_blocks_;
  }

 private:
  using b2j_t = std::unordered_map<hashable_type, std::vector<size_t>>;
  using junk_set_t = std::unordered_set<hashable_type>;

  void chain_b() {
    size_t index=0;
   
    // Counting occurences
    b2j_.clear();
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
    if (auto_junk_ && auto_junk_minsize_ <= b_.size()) {
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

protected:
  T a_;
  T b_;
  junk_function_type is_junk_ = NoJunk<hashable_type>;
private:
  bool auto_junk_ = true;
  std::size_t auto_junk_minsize_ = 200u;
  b2j_t b2j_;

  // Cache to avoid reallocations
  std::vector<size_t> j2len_;
  std::vector<std::pair<size_t, size_t>> j2_values_to_affect_;
  std::vector<std::pair<size_t, size_t>> j2_values_to_erase_;

  junk_set_t junk_set_;
  junk_set_t popular_set_;
protected:
  std::unique_ptr<match_list_t> matching_blocks_;
};

template <class T> auto MakeSequenceMatcher(
  T const& a
  , T const& b
  , typename SequenceMatcher<T>::junk_function_type is_junk = NoJunk<typename SequenceMatcher<T>::hashable_type>
  , bool auto_junk = true 
)
-> SequenceMatcher<T> 
{ 
  return SequenceMatcher<T>(a, b, is_junk, auto_junk);
}

}  // namespace difflib

#ifdef DIFFLIB_ENABLE_EXTERN_MACROS
#  define DIFFLIB_MAKE_EXTERN_FOR_TYPE(A)\
      namespace difflib {\
      extern template class SequenceMatcher<A>;\
      extern template SequenceMatcher<A> MakeSequenceMatcher<A>(A const&, A const&, typename SequenceMatcher<A>::junk_function_type, bool);\
      }  // namespace difflib

#  define DIFFLIB_INSTANTIATE_FOR_TYPE(A)\
      namespace difflib {\
      template class SequenceMatcher<A>;\
      template SequenceMatcher<A> MakeSequenceMatcher<A>(A const&, A const&, typename SequenceMatcher<A>::junk_function_type, bool);\
      }  // namespace difflib
#endif

#endif  // __DIFFLIB__
