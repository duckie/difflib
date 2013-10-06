#include <difflib.h>
#include <iostream>
#include <string>
#include <vector>

struct Useless {};

int main() {
  std::cout << difflib::is_hashable_sequence<std::string>::value << std::endl;
  std::cout << difflib::is_hashable_sequence< std::vector<std::string> >::value << std::endl;
  std::cout << difflib::is_hashable_sequence<Useless>::value << std::endl;
  
  std::string a = "test";
  std::string b = "test2";

  auto foo = difflib::MakeSequenceMatcher(a,b);
  // auto bar = difflib::MakeSequenceMatcher(Useless(), Useless());  // Wouldnt compile because of static_assert
  return 0;
}
