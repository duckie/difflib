#include <difflib.h>
#include <iostream>
#include <string>
#include <vector>

struct Useless {};

static_assert(!difflib::is_standard_iterable<Useless>::value,"Check that Useless is not iterable");
static_assert(!difflib::is_hashable_sequence<Useless>::value,"Check that Useless is not a hashable sequence");

int main() {
  std::cout << difflib::is_hashable_sequence<std::string>::value << std::endl;
  std::cout << difflib::is_hashable_sequence< std::vector<std::string> >::value << std::endl;
  std::cout << difflib::is_hashable_sequence<Useless>::value << std::endl;
  
  std::string a = "test";
  std::string b = "test2";

  std::vector<std::string> va = {"1", "2", "3"};
  std::vector<std::string> vb = {"1", "4", "3"};

  auto foo = difflib::MakeSequenceMatcher(a,b);
  //auto vfoo = difflib::MakeSequenceMatcher(va,vb);
  return 0;
}
