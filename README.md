# C++11 Difflib

### Explanation

This is a port of the great difflib tool in python into C++11.

For the moment, the library is made of one templated header. Once the implementation is complete, it will be splitted in difflib.h and difflib\_impl.h to ease the use of extern templates in order to minimize binary size and speed up compilation time.

### To compile and test

```bash
mkdir build
cd build
cmake ../
make
./test/difflib-test
```
