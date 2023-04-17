cmake_minimum_required(VERSION 3.18)

include_directories(include)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Catch2 2 REQUIRED)
project(IncrementalC 
    LANGUAGES 
        CXX
)


add_executable(stage_1_tests tests/stage_1_tests.cpp lexer.cpp parse.cpp ast.cpp)
set_target_properties(stage_1_tests PROPERTIES SUFFIX ".out")
target_link_libraries(stage_1_tests PRIVATE Catch2::Catch2WithMain)

add_executable(stage_2_tests tests/stage_2_tests.cpp lexer.cpp parse.cpp ast.cpp)
set_target_properties(stage_2_tests PROPERTIES SUFFIX ".out")
target_link_libraries(stage_2_tests PRIVATE Catch2::Catch2WithMain)

add_executable(stage_3_tests tests/stage_3_tests.cpp lexer.cpp parse.cpp ast.cpp)
set_target_properties(stage_3_tests PROPERTIES SUFFIX ".out")
target_link_libraries(stage_3_tests PRIVATE Catch2::Catch2WithMain)

add_executable(incremental_c incremental_c.cpp lexer.cpp parse.cpp ast.cpp )
set_target_properties(incremental_c PROPERTIES SUFFIX ".out")
