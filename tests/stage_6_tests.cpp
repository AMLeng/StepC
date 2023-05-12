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


TEST_CASE("empty expression stmt"){
    auto ss = std::stringstream(
R"(int main(){
    long int a;
    ;;
    return 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse multiple scope"){
    auto ss = std::stringstream(
R"(int main(){
    long int a;
    {
        int a;
        {
            short b = 3;
        }
    }
    return 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("if ident"){
    auto ss = std::stringstream(
R"(int main(){
    long int if = 3;
    return f;
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
    //program_pointer->pretty_print(0);
}
TEST_CASE("if missing condition"){
    auto ss = std::stringstream(
R"(int main(){
    long int a = 3;
    if
    {
         a = a*3;
    }
    return a;
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
    //program_pointer->pretty_print(0);
}
TEST_CASE("if missing condition 2"){
    auto ss = std::stringstream(
R"(int main(){
    long int a = 3;
    if a
    {
         a = a*3;
    }
    return a;
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
    //program_pointer->pretty_print(0);
}

TEST_CASE("parse if"){
    auto ss = std::stringstream(
R"(int main(){
    long int a = 3;
    if(a)
    {
         a = a*3;
    }
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse non-compound if"){
    auto ss = std::stringstream(
R"(int main(){
    long int a = 3;
    if(a)
         a = 1;
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse if else non-compound"){
    auto ss = std::stringstream(
R"(int main(){
    long int a = 3;
    if(a)
         a = 1;
    else
        a = 2;
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}

//Tests exclusive to this stage (e.g. that the compiler fails on things that haven't been implemented yet)
//Tests which use structure that will be refactored later should not be here
//Since those tests should be updated during refactoring
#ifdef STAGE_6

#endif
