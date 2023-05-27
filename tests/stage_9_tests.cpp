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


TEST_CASE("parse function decl with param names"){
    auto ss = std::stringstream(
R"(
int a,b(int p1, double p2);
int main(){
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse function decl with param names 2"){
    auto ss = std::stringstream(
R"(
int a,b(int b, double a);
int main(){
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse function decl with same param names "){
    auto ss = std::stringstream(
R"(
int a,b(int a, double a);
int main(){
    return a;
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), sem_error::STError);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse function decl missing param names"){
    auto ss = std::stringstream(
R"(
int a,b(int , double );
int main(){
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse function decl missing param names 2"){
    auto ss = std::stringstream(
R"(
int a,b(int b, double);
int main(){
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse function decl void type"){
    auto ss = std::stringstream(
R"(
void b(int b, double);
int main(){
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("error no void variable"){
    auto ss = std::stringstream(
R"(int main(){
    void b;
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), sem_error::TypeError);
}
TEST_CASE("error multiple function decl"){
    auto ss = std::stringstream(
R"(
void b(int);
void b(char);
int main(){
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::STError);
}
TEST_CASE("error multiple function decl 2"){
    auto ss = std::stringstream(
R"(
void b(int a);
int b(int a);
int main(){
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::STError);
}
TEST_CASE("error function decl not at global scope"){
    auto ss = std::stringstream(
R"(
int main(){
    int b(int a);
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::STError);
}


//Tests exclusive to this stage (e.g. that the compiler fails on things that haven't been implemented yet)
//Tests which use structure that will be refactored later should not be here
//Since those tests should be updated during refactoring
#ifdef STAGE_9

#endif
