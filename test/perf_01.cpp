#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <fstream>
#include <streambuf>
#include <chrono>
#include <difflib.h>

int main(int argc, char * argv[]) {
  if (argc < 3) return 1;  // Need two arguments

  std::ifstream file_a(argv[1]);
  std::string a((std::istreambuf_iterator<char>(file_a)), std::istreambuf_iterator<char>());

  std::ifstream file_b(argv[2]);
  std::string b((std::istreambuf_iterator<char>(file_b)), std::istreambuf_iterator<char>());

  double ratio = 0;

  for (int i = 0; i < 10000; ++i) {
    auto matcher= difflib::MakeSequenceMatcher<>(a,b);
    ratio = matcher.ratio();
  }

  std::cout<< ratio <<std::endl;

  return 0;
}
