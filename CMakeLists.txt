cmake_minimum_required(VERSION 3.10)
project(lighthouse2tools C CXX)

include(CheckCXXSourceCompiles)
check_cxx_source_compiles("int main() { __builtin_popcount(0); return 0; }" HAS_BUILTIN_POPCOUNT)
if(HAS_BUILTIN_POPCOUNT)
    add_definitions(-DHAS_BUILTIN_POPCOUNT)
endif()

add_subdirectory(src)
