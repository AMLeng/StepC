#include "catch2/catch.hpp"
#include "lexer.h"
#include "lexer_error.h"
#include "parse_error.h"
#include "parse.h"
#include <iostream>
#include <sstream>
#include <utility>


//Lexer unit tests
TEST_CASE("recognizes_minus"){
    auto ss = std::stringstream("-3");
    lexer::Lexer l(ss);
    REQUIRE(l.get_token().type == lexer::Token::TokenType::Minus);
}
TEST_CASE("recognizes_bitwise_not"){
    auto ss = std::stringstream("~5");
    lexer::Lexer l(ss);
    REQUIRE(l.get_token().type == lexer::Token::TokenType::BitwiseNot);
}
TEST_CASE("recognizes_logical_not"){
    auto ss = std::stringstream("!7");
    lexer::Lexer l(ss);
    REQUIRE(l.get_token().type == lexer::Token::TokenType::Not);
}

//Parser tests
TEST_CASE("parse_error_stage_two"){
    auto ss = std::stringstream(
R"(int main(){
    return --3-;
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
}

TEST_CASE("valid parse_stage_two"){
    auto ss = std::stringstream(
R"(int main(){
    return -~!3;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}

//Tests exclusive to this stage (e.g. that the compiler fails on things that haven't been implemented yet)
//Tests which use structure that will be refactored later should not be here
//Since those tests should be updated during refactoring
#ifdef STAGE_2

#endif
