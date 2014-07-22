// The MIT License (MIT)
// 
// Copyright (c) 2014 Jean-Bernard Jansen
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

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
  

  for (int i = 0; i < 1000; ++i) 
  {
    auto matcher= difflib::MakeSequenceMatcher<>(a,b);
    ratio = matcher.ratio();
  }

  std::cout<< ratio <<std::endl;

  return 0;
}
