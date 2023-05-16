cmake_minimum_required(VERSION 3.18)

include_directories(include)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Catch2 2 REQUIRED)
project(IncrementalC 
    LANGUAGES 
        CXX
)

add_library(core lex/lexer.cpp parse/parse.cpp parse/ast_construct.cpp parse/type.cpp parse/ast_pretty_print.cpp sem/ast_analyze.cpp codegen/ast_codegen.cpp codegen/context.cpp codegen/basic_block.cpp)

add_executable(stage_1_tests tests/stage_1_tests.cpp)
set_target_properties(stage_1_tests PROPERTIES SUFFIX ".out")
target_link_libraries(stage_1_tests PRIVATE Catch2::Catch2WithMain core)

add_executable(stage_2_tests tests/stage_2_tests.cpp)
set_target_properties(stage_2_tests PROPERTIES SUFFIX ".out")
target_link_libraries(stage_2_tests PRIVATE Catch2::Catch2WithMain core)

add_executable(stage_3_tests tests/stage_3_tests.cpp)
set_target_properties(stage_3_tests PROPERTIES SUFFIX ".out")
target_link_libraries(stage_3_tests PRIVATE Catch2::Catch2WithMain core)

add_executable(stage_4_tests tests/stage_4_tests.cpp)
set_target_properties(stage_4_tests PROPERTIES SUFFIX ".out")
target_link_libraries(stage_4_tests PRIVATE Catch2::Catch2WithMain core)

add_executable(stage_5_tests tests/stage_5_tests.cpp)
set_target_properties(stage_5_tests PROPERTIES SUFFIX ".out")
target_link_libraries(stage_5_tests PRIVATE Catch2::Catch2WithMain core)

add_executable(stage_6_tests tests/stage_6_tests.cpp)
set_target_properties(stage_6_tests PROPERTIES SUFFIX ".out")
target_link_libraries(stage_6_tests PRIVATE Catch2::Catch2WithMain core)

add_executable(step_c step_c.cpp)
target_link_libraries(step_c PRIVATE core)
set_target_properties(step_c PROPERTIES SUFFIX ".out")
