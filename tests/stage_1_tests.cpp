#include "catch2/catch.hpp"
#include "lexer.h"
#include "lexer_error.h"
#include "parse_error.h"
#include "parse.h"
#include <iostream>
#include <sstream>
#include <utility>


//Lexer unit tests
TEST_CASE("recognizes_empty_input_stream"){
    auto ss = std::stringstream("");
    lexer::Lexer l(ss);
    REQUIRE(l.get_token().type == lexer::Token::TokenType::END);
    REQUIRE(l.get_location().first == 1);
    REQUIRE(l.get_location().second == 1);
}
TEST_CASE("recognizes_keyword_int"){
    auto ss = std::stringstream("int");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == lexer::Token::TokenType::Keyword);
    REQUIRE(t.value == "int");
    REQUIRE(l.get_location().first == 1);
    REQUIRE(l.get_location().second == 4);
}
TEST_CASE("recognizes_keyword_return"){
    auto ss = std::stringstream("return");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == lexer::Token::TokenType::Keyword);
    REQUIRE(t.value == "return");
    REQUIRE(l.get_location().first == 1);
    REQUIRE(l.get_location().second == 7);
}
TEST_CASE("recognizes_identifier_main"){
    auto ss = std::stringstream("main");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == lexer::Token::TokenType::Identifier);
    REQUIRE(t.value == "main");
}
TEST_CASE("decimal_literal_no_suffix"){
    auto ss = std::stringstream("25");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == lexer::Token::TokenType::IntegerLiteral);
    REQUIRE(t.value == "25");
}
TEST_CASE("decimal_literal_u"){
    auto ss = std::stringstream("2532u");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == lexer::Token::TokenType::IntegerLiteral);
    REQUIRE(t.value == "2532u");
}
TEST_CASE("decimal_literal_no_ll"){
    auto ss = std::stringstream("3993993ll");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == lexer::Token::TokenType::IntegerLiteral);
    REQUIRE(t.value == "3993993ll");
}
TEST_CASE("decimal_literal_uLL"){
    auto ss = std::stringstream("9230uLL");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == lexer::Token::TokenType::IntegerLiteral);
    REQUIRE(t.value == "9230uLL");
}
TEST_CASE("decimal_literal_llU"){
    auto ss = std::stringstream("4435llU");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == lexer::Token::TokenType::IntegerLiteral);
    REQUIRE(t.value == "4435llU");
}
TEST_CASE("decimal_literal_lLU"){
    auto ss = std::stringstream("5lLU");
    lexer::Lexer l(ss);
    l.get_token();
    REQUIRE(l.get_location().second == 3);
    auto t = l.get_token();
    REQUIRE(t.value == "LU");
}
TEST_CASE("decimal_literal_uLl"){
    auto ss = std::stringstream("5uLl");
    lexer::Lexer l(ss);
    l.get_token();
    REQUIRE(l.get_location().second == 4);
    auto t = l.get_token();
    REQUIRE(t.value =="l");
}
TEST_CASE("octal_literal"){
    auto ss = std::stringstream("025");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == lexer::Token::TokenType::IntegerLiteral);
    REQUIRE(t.value == "025");
}
TEST_CASE("invalid_octal_literal"){
    auto ss = std::stringstream("028");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(l.get_token(), lexer_error::InvalidLiteral);
    REQUIRE(l.get_location().first == 1);
    REQUIRE(l.get_location().second == 3);
}
TEST_CASE("invalid_octal_float_literal"){
    auto ss = std::stringstream("027.5");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(l.get_token(), lexer_error::InvalidLiteral);
    REQUIRE(l.get_location().first == 1);
    REQUIRE(l.get_location().second == 4);
}
TEST_CASE("hex_literal"){
    auto ss = std::stringstream("0x25afeF");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == lexer::Token::TokenType::IntegerLiteral);
    REQUIRE(t.value == "0x25afeF");
}
TEST_CASE("invalid_hex_literal"){
    auto ss = std::stringstream("0xg28");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(l.get_token(), lexer_error::InvalidLiteral);
    REQUIRE(l.get_location().first == 1);
    REQUIRE(l.get_location().second == 3);
}

TEST_CASE("whitespace_octal_literal"){
    auto ss = std::stringstream(
R"(     
          
        025)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == lexer::Token::TokenType::IntegerLiteral);
    REQUIRE(t.value == "025");
    REQUIRE(l.get_location().first == 3);
    REQUIRE(l.get_location().second == 12);
}
TEST_CASE("whitespace_invalid_octal_literal"){
    auto ss = std::stringstream(R"(  
        
        028)");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(l.get_token(), lexer_error::InvalidLiteral);
    REQUIRE(l.get_location().first == 3);
    REQUIRE(l.get_location().second == 11);
}

//End lexer unit tests
//Parser tests (black box for whole system)
TEST_CASE("parse_error_stage_one"){
    auto ss = std::stringstream(
R"(int main(){
    return 3
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
    //Somewhat awkwardly, the lexer has already finished consuming '}' and is at the finished state
    //On the other hand, this should probably be fine; we're only trying to test the parser, so the lexer's state
    //Should not matter to us
}

//Tests exclusive to this stage (e.g. that the compiler fails on things that haven't been implemented yet)
#ifdef STAGE_1

TEST_CASE("hex_float_undefined"){
    auto ss = std::stringstream("0xf28.35");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(l.get_token(), lexer_error::NotImplemented);
    REQUIRE(l.get_location().first == 1);
    REQUIRE(l.get_location().second == 6);
}
TEST_CASE("dec_float_undefined"){
    auto ss = std::stringstream("28.35");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(l.get_token(), lexer_error::NotImplemented);
    REQUIRE(l.get_location().first == 1);
    REQUIRE(l.get_location().second == 3);
}
#endif
