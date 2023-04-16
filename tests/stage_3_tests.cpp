#include "catch2/catch.hpp"
#include "lexer.h"
#include "lexer_error.h"
#include "parse_error.h"
#include "parse.h"
#include <iostream>
#include <sstream>
#include <utility>


//Lexer unit tests
TEST_CASE("recognizes_plus"){
    auto ss = std::stringstream("2+3");
    lexer::Lexer l(ss);
    REQUIRE(l.get_token().type == lexer::Token::TokenType::IntegerLiteral);
    REQUIRE(l.get_token().type == lexer::Token::TokenType::Plus);
    REQUIRE(l.get_token().type == lexer::Token::TokenType::IntegerLiteral);
}
TEST_CASE("recognizes_mult"){
    auto ss = std::stringstream("3*5");
    lexer::Lexer l(ss);
    REQUIRE(l.get_token().type == lexer::Token::TokenType::IntegerLiteral);
    REQUIRE(l.get_token().type == lexer::Token::TokenType::Mult);
    REQUIRE(l.get_token().type == lexer::Token::TokenType::IntegerLiteral);
}
TEST_CASE("recognizes_logical_not"){
    auto ss = std::stringstream("5/4");
    lexer::Lexer l(ss);
    REQUIRE(l.get_token().type == lexer::Token::TokenType::IntegerLiteral);
    REQUIRE(l.get_token().type == lexer::Token::TokenType::Div);
    REQUIRE(l.get_token().type == lexer::Token::TokenType::IntegerLiteral);
}
//Tests exclusive to this stage (e.g. that the compiler fails on things that haven't been implemented yet)
//Tests which use structure that will be refactored later should not be here
//Since those tests should be updated during refactoring
#ifdef STAGE_3

#endif
