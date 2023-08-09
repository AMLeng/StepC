#include "catch2/catch.hpp"
#include "lexer.h"
#include "parse.h"
#include "type.h"
#include "lexer_error.h"
#include "parse_error.h"
#include "sem_error.h"
#include <iostream>
#include <sstream>
#include <utility>

TEST_CASE("line splicing"){
    auto ss = std::stringstream(
R"(
int ma\
in(){
    ch\
ar (*a)[25] = &"This is a string literal";
}

)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("trigraphs"){
    auto ss = std::stringstream(
R"(
int main(){
    char (*a)??(25??) = &"This is a string literal";
    return ??-3??'4;
}

)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("trigraphs with splice"){
    auto ss = std::stringstream(
R"(
int main(){
    char (*a)??(\
25] = &"This is a string literal";
}

)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("object macros"){
    auto ss = std::stringstream(
R"(
#define Blah main
#define Foo int
Foo Blah(){
    Foo x = 3;
    return x;
}

)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}

TEST_CASE("object macros multitoken"){
    auto ss = std::stringstream(
R"(
#define Blah long long
#define Foo int
Foo main(){
    Blah y = 4;
    Blah Foo x = 3;
    return sizeof(y) - sizeof(x);
}

)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
