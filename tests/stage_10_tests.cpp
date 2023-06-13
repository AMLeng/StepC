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


TEST_CASE("parse pointer decls"){
    auto ss = std::stringstream(
R"(
int main(){
    int* a;
    double** b;
    int (*c)(int, int);
    return 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("parse complex decl 1"){
    auto ss = std::stringstream(
R"(
int main(){
    void* (*a)(int *,double,...);
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("parse complex decl 2"){
    auto ss = std::stringstream(
R"(
int main(){
    int(b)(int*(*)(int));
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("parse complex decl 3"){
    auto ss = std::stringstream(
R"(
int main(){
    int(*(*c)(void))(int*(*)(int));
})");
    //Pointer to a function on (void) returning a pointer to
    //  "a function taking in an int*(*)(int) and returning an int"
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("parse complex decl 4"){
    auto ss = std::stringstream(
R"(
int main(){
    int**(*(**c)())(int);
})");
    //Pointer to pointer to a function returning a pointer to
    //  "a function taking in an int and returning an int**"
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("invalid decl 1"){
    auto ss = std::stringstream(
R"(
int main(){
    void* (a*)(int *,double,...);
})");
    //Identifier in incorrect spot relative to pointer *
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
}
TEST_CASE("invalid decl 2"){
    auto ss = std::stringstream(
R"(
int main(){
    int(int*(*)(int)) (*c)(void);
})");
    //Identifier must come before any function arg types
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
}


//Tests exclusive to this stage (e.g. that the compiler fails on things that haven't been implemented yet)
//Tests which use structure that will be refactored later should not be here
//Since those tests should be updated during refactoring
#ifdef STAGE_10

#endif
