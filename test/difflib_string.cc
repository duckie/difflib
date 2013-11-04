#include <string>
#include <vector>

#define DIFFLIB_ENABLE_EXTERN_MACROS
#include <difflib.h>

DIFFLIB_INSTANTIATE_FOR_TYPE(std::string);
DIFFLIB_INSTANTIATE_FOR_TYPE(std::vector<std::string>);
