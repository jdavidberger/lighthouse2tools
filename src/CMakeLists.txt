add_library(lh2tools lfsr.cc lfsr.h lh2polys.cc)

set(CMAKE_CXX_STANDARD 17)

add_executable(find_best_poly find_best_poly.cc)
add_executable(r2x0t_decode r2x0t_decode.c)
target_link_libraries(find_best_poly lh2tools)

add_executable(iterate_str iterate_str.cc)
target_link_libraries(iterate_str lh2tools)