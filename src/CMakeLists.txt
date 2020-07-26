add_library(lh2tools lfsr.cc ../include/lfsr.h lh2polys.cc)

target_include_directories(lh2tools PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/>
        $<INSTALL_INTERFACE:include/>
        )

set(CMAKE_CXX_STANDARD 17)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  
add_executable(find_best_poly find_best_poly.cc)
add_executable(r2x0t_decode r2x0t_decode.c)
target_link_libraries(find_best_poly lh2tools)

add_executable(iterate_str iterate_str.cc)
target_link_libraries(iterate_str lh2tools)

endif()
