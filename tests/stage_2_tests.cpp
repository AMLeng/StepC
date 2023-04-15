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
//Tests exclusive to this stage (e.g. that the compiler fails on things that haven't been implemented yet)
#ifdef STAGE_2

#endif
