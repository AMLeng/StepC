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


//Lexer unit tests
TEST_CASE("int decl"){
    auto ss = std::stringstream(
R"(int a;)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == token::TokenType::Keyword);
    REQUIRE(t.value == "int");
    t = l.get_token();
    REQUIRE(t.type == token::TokenType::Identifier);
    REQUIRE(t.value == "a");
}
TEST_CASE("int def"){
    auto ss = std::stringstream(
R"(int a = 3;)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == token::TokenType::Keyword);
    REQUIRE(t.value == "int");
    t = l.get_token();
    REQUIRE(t.type == token::TokenType::Identifier);
    REQUIRE(t.value == "a");
    t = l.get_token();
    REQUIRE(t.type == token::TokenType::Assign);
    t = l.get_token();
    REQUIRE(t.type == token::TokenType::IntegerLiteral);
    REQUIRE(t.value == "3");
}
TEST_CASE("ull decl"){
    auto ss = std::stringstream(
R"(unsigned long long int a;)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == token::TokenType::Keyword);
    REQUIRE(t.value == "unsigned");
    t = l.get_token();
    REQUIRE(t.type == token::TokenType::Keyword);
    REQUIRE(t.value == "long");
    t = l.get_token();
    REQUIRE(t.type == token::TokenType::Keyword);
    REQUIRE(t.value == "long");
    t = l.get_token();
    REQUIRE(t.type == token::TokenType::Keyword);
    REQUIRE(t.value == "int");
    t = l.get_token();
    REQUIRE(t.type == token::TokenType::Identifier);
    REQUIRE(t.value == "a");
}
TEST_CASE("signed short def"){
    auto ss = std::stringstream(
R"(signed short a = 3.5;)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == token::TokenType::Keyword);
    REQUIRE(t.value == "signed");
    t = l.get_token();
    REQUIRE(t.type == token::TokenType::Keyword);
    REQUIRE(t.value == "short");
    t = l.get_token();
    REQUIRE(t.type == token::TokenType::Identifier);
    REQUIRE(t.value == "a");
    t = l.get_token();
    REQUIRE(t.type == token::TokenType::Assign);
}
//Parser tests
//Type deduction tests

TEST_CASE("parse shorts"){
    auto ss = std::stringstream(
R"(signed short)");
    lexer::Lexer l(ss);

}
TEST_CASE("parse unsigned long long"){
    auto ss = std::stringstream(
R"(long unsigned long int)");
    lexer::Lexer l(ss);

}
TEST_CASE("parse unsigned long long 2"){
    auto ss = std::stringstream(
R"(unsigned long long)");
    lexer::Lexer l(ss);

}
TEST_CASE("parse short"){
    auto ss = std::stringstream(
R"(int short)");
    lexer::Lexer l(ss);

}
TEST_CASE("parse error program with decl"){
    auto ss = std::stringstream(
R"(int main(){
    long char a;
    return 5;
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l),sem_error::TypeError);
    //program_pointer->pretty_print(0);
}
/*TEST_CASE("parse program with decl"){
    auto ss = std::stringstream(
R"(int main(){
    long unsigned a;
    return 0;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}*/

/*TEST_CASE("parse full program"){
    auto ss = std::stringstream(
R"(int main(){
    long unsigned a = 5;
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}*/
/*TEST_CASE("parse_error_stage_two"){
    auto ss = std::stringstream(
R"(int main(){
    return --3-;
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
}*/


//Tests exclusive to this stage (e.g. that the compiler fails on things that haven't been implemented yet)
//Tests which use structure that will be refactored later should not be here
//Since those tests should be updated during refactoring
#ifdef STAGE_5

#endif
