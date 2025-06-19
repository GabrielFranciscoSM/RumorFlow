cmake_minimum_required(VERSION 3.31)

PROJECT(mh_p1 C CXX)

file(GLOB C_SOURCES
  "src/*.cpp"
  "src/P1/*.cpp"
  "src/P2/*.cpp"
  "src/P3/*.cpp"
)
INCLUDE_DIRECTORIES("common" "inc" "inc/P1" "inc/P2" "inc/P3")

add_library(cec17_test_func SHARED code/cec17_test_func.c code/cec17.c)
target_include_directories(cec17_test_func PUBLIC code)
set_target_properties(cec17_test_func PROPERTIES LINKER_LANGUAGE C)

add_executable(main main.cpp ${C_SOURCES})
target_link_libraries(main PRIVATE cec17_test_func)
target_include_directories(main PRIVATE code)
