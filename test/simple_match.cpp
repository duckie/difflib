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
  
  auto matcher= difflib::MakeSequenceMatcher<>(a,b);
  auto match_list = matcher.get_matching_blocks();
  ratio = matcher.ratio();

  for(auto const& triple : match_list) {
    std::cout 
      << "a=" << std::get<0>(triple) 
      << ", b=" << std::get<1>(triple)
      << ", s=" << std::get<2>(triple)
      << "\n";
  }

  std::cout<< ratio <<std::endl;
  //std::cout<< nb_match <<std::endl;

  return 0;
}
