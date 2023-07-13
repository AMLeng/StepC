cmake_minimum_required(VERSION 3.18)

include_directories(include)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Catch2 2 REQUIRED)
project(IncrementalC 
    LANGUAGES 
        CXX
)
set(CMAKE_CXX_FLAGS "-Wno-psabi")
add_library(type
    type/type_basic.cpp type/type_func.cpp type/type_derived.cpp 
    type/type_pointer.cpp type/type_array.cpp type/type_struct.cpp type/type.cpp 
)
add_library (codegen_utils
    codegen/codegen_utility.cpp codegen/binary_operator_codegen.cpp
)
add_library(core lex/lexer.cpp 
    parse/parse.cpp parse/parse_decl.cpp parse/ast_construct.cpp parse/ast_pretty_print.cpp 
    sem/ast_analyze.cpp sem/symbol.cpp
    codegen/ast_codegen.cpp codegen/context.cpp codegen/basic_block.cpp )
target_link_libraries(core type codegen_utils)

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

add_executable(stage_7_tests tests/stage_7_tests.cpp)
set_target_properties(stage_7_tests PROPERTIES SUFFIX ".out")
target_link_libraries(stage_7_tests PRIVATE Catch2::Catch2WithMain core)

add_executable(stage_8_tests tests/stage_8_tests.cpp)
set_target_properties(stage_8_tests PROPERTIES SUFFIX ".out")
target_link_libraries(stage_8_tests PRIVATE Catch2::Catch2WithMain core)

add_executable(stage_9_tests tests/stage_9_tests.cpp)
set_target_properties(stage_9_tests PROPERTIES SUFFIX ".out")
target_link_libraries(stage_9_tests PRIVATE Catch2::Catch2WithMain core)

add_executable(stage_10_tests tests/stage_10_tests.cpp)
set_target_properties(stage_10_tests PROPERTIES SUFFIX ".out")
target_link_libraries(stage_10_tests PRIVATE Catch2::Catch2WithMain core)

add_executable(stage_11_tests tests/stage_11_tests.cpp)
set_target_properties(stage_11_tests PROPERTIES SUFFIX ".out")
target_link_libraries(stage_11_tests PRIVATE Catch2::Catch2WithMain core)

add_executable(step_c step_c.cpp)
target_link_libraries(step_c PRIVATE core)
set_target_properties(step_c PROPERTIES SUFFIX ".out")
