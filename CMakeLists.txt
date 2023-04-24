cmake_minimum_required(VERSION 3.18)

include_directories(include)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Catch2 2 REQUIRED)
project(IncrementalC 
    LANGUAGES 
        CXX
)

add_library(lex lex/lexer.cpp)
add_library(parse parse/parse.cpp parse/ast_sem.cpp)
add_library(codegen codegen/ast_codegen.cpp)

add_executable(stage_1_tests tests/stage_1_tests.cpp)
set_target_properties(stage_1_tests PROPERTIES SUFFIX ".out")
target_link_libraries(stage_1_tests PRIVATE Catch2::Catch2WithMain lex parse codegen)

add_executable(stage_2_tests tests/stage_2_tests.cpp)
set_target_properties(stage_2_tests PROPERTIES SUFFIX ".out")
target_link_libraries(stage_2_tests PRIVATE Catch2::Catch2WithMain lex parse codegen)

add_executable(stage_3_tests tests/stage_3_tests.cpp)
set_target_properties(stage_3_tests PROPERTIES SUFFIX ".out")
target_link_libraries(stage_3_tests PRIVATE Catch2::Catch2WithMain lex parse codegen)

add_executable(stage_4_tests tests/stage_4_tests.cpp)
set_target_properties(stage_4_tests PROPERTIES SUFFIX ".out")
target_link_libraries(stage_4_tests PRIVATE Catch2::Catch2WithMain lex parse codegen)

add_executable(step_c step_c.cpp)
target_link_libraries(step_c PRIVATE lex parse codegen)
set_target_properties(step_c PROPERTIES SUFFIX ".out")
