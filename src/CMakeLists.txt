add_library(lh2tools lfsr.cc lfsr.h lh2polys.cc)

set(CMAKE_CXX_STANDARD 17)

add_executable(find_best_poly find_best_poly.cc)
target_link_libraries(find_best_poly lh2tools)