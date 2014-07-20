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
  std::size_t nb_match = 0;
  

  for (int i = 0; i < 1000; ++i) 
  {
    auto matcher= difflib::MakeSequenceMatcher<>(a,b);
    //auto matcher= difflib::MakeSequenceMatcher<>(a,b, difflib::NoJunk<char>, true);
    ratio = matcher.ratio();
    //auto match_list = matcher.get_matching_blocks();
    //nb_match = matcher.get_matching_blocks().size();
  }
  //
  //for(auto const& triple : match_list) {
    //std::cout 
      //<< "a=" << std::get<0>(triple) 
      //<< ", b=" << std::get<1>(triple)
      //<< ", s=" << std::get<2>(triple)
      //<< "\n";
  //}

  std::cout<< ratio <<std::endl;
  //std::cout<< nb_match <<std::endl;

  return 0;
}
