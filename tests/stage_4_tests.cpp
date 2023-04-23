#include "catch2/catch.hpp"
#include "lexer.h"
#include "lexer_error.h"
#include "parse_error.h"
#include "parse.h"
#include <iostream>
#include <sstream>
#include <utility>


//Lexer unit tests
TEST_CASE("dec_float"){
    auto ss = std::stringstream(
R"(28.5)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == token::TokenType::FloatLiteral);
    REQUIRE(t.value == "28.5");
}
TEST_CASE("leading_zero_dec_float"){
    auto ss = std::stringstream(
R"(028.5)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == token::TokenType::FloatLiteral);
    REQUIRE(t.value == "028.5");
    REQUIRE(l.get_location().first == 1);
    REQUIRE(l.get_location().second == 6);
}
TEST_CASE("leading_zero_hex_float"){
    auto ss = std::stringstream(
R"(0x028f.5p5)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == token::TokenType::FloatLiteral);
    REQUIRE(t.value == "0x028f.5p5");
}
TEST_CASE("hex_float"){
    auto ss = std::stringstream(
R"(0x28f.5p5)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == token::TokenType::FloatLiteral);
    REQUIRE(t.value == "0x28f.5p5");
}
TEST_CASE("hex_float_sign"){
    auto ss = std::stringstream(
R"(0x28f.5p-5)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == token::TokenType::FloatLiteral);
    REQUIRE(t.value == "0x28f.5p-5");
}
TEST_CASE("hex_float_l"){
    auto ss = std::stringstream(
R"(0x28f.5p5l)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == token::TokenType::FloatLiteral);
    REQUIRE(t.value == "0x28f.5p5l");
}
TEST_CASE("hex_float_L"){
    auto ss = std::stringstream(
R"(0x28f.5p5L)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == token::TokenType::FloatLiteral);
    REQUIRE(t.value == "0x28f.5p5L");
}
TEST_CASE("hex_float_f"){
    auto ss = std::stringstream(
R"(0x28f.5p5f)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == token::TokenType::FloatLiteral);
    REQUIRE(t.value == "0x28f.5p5f");
}
TEST_CASE("hex_float_F"){
    auto ss = std::stringstream(
R"(0x28f.5p5F)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == token::TokenType::FloatLiteral);
    REQUIRE(t.value == "0x28f.5p5F");
}
TEST_CASE("dec_float_no_int"){
    auto ss = std::stringstream(
R"(.35)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == token::TokenType::FloatLiteral);
    REQUIRE(t.value == ".35");
}
TEST_CASE("dec_float_e"){
    auto ss = std::stringstream(
R"(0.35e10l)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == token::TokenType::FloatLiteral);
    REQUIRE(t.value == "0.35e10l");
}
TEST_CASE("dec_float_E"){
    auto ss = std::stringstream(
R"(0.35E10f)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == token::TokenType::FloatLiteral);
    REQUIRE(t.value == "0.35E10f");
}
TEST_CASE("dec_float_E_sign"){
    auto ss = std::stringstream(
R"(0.35E+10f)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE(t.type == token::TokenType::FloatLiteral);
    REQUIRE(t.value == "0.35E+10f");
}
TEST_CASE("invalid_hex_float_no_exponent"){
    auto ss = std::stringstream(
R"(0x28.f)");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(l.get_token(), lexer_error::InvalidLiteral);
}
TEST_CASE("invalid_hex_float_no_exponent_num"){
    auto ss = std::stringstream(
R"(0x28.fp)");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(l.get_token(), lexer_error::InvalidLiteral);
}
TEST_CASE("invalid_hex_float_no_exponent_suffix"){
    auto ss = std::stringstream(
R"(0x28.fpf)");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(l.get_token(), lexer_error::InvalidLiteral);
}
//End lexer unit tests

//Parser tests, type checking
TEST_CASE("parse_hex_float_l"){
    auto ss = std::stringstream(
R"(0x28f.5p5l)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(c.type == "long double");
    REQUIRE(c.literal == "0x28f.5p5");
}
TEST_CASE("parse_hex_float_L"){
    auto ss = std::stringstream(
R"(0x28f.5p5L)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(c.type == "long double");
    REQUIRE(c.literal == "0x28f.5p5");
}
TEST_CASE("parse_hex_float_f"){
    auto ss = std::stringstream(
R"(0x28f.5p5f)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(c.type == "float");
    REQUIRE(c.literal == "0x28f.5p5");
}
TEST_CASE("parse_hex_float_F"){
    auto ss = std::stringstream(
R"(0x28f.5p5F)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(c.type == "float");
    REQUIRE(c.literal == "0x28f.5p5");
}
TEST_CASE("parse_hex_float"){
    auto ss = std::stringstream(
R"(0x28f.5p5)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(c.type == "double");
    REQUIRE(c.literal == "0x28f.5p5");
}
TEST_CASE("parse_decimal_U"){
    auto ss = std::stringstream("2532U");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(c.type == "unsigned int");
    REQUIRE(c.literal == "2532");
}
TEST_CASE("parse_decimal_u"){
    auto ss = std::stringstream("2532u");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(c.type == "unsigned int");
    REQUIRE(c.literal == "2532");
}
TEST_CASE("parse_decimal_ull"){
    auto ss = std::stringstream("2532ull");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(c.type == "unsigned long long int");
    REQUIRE(c.literal == "2532");
}
TEST_CASE("parse_decimal_LLu"){
    auto ss = std::stringstream("2532LLu");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(c.type == "unsigned long long int");
    REQUIRE(c.literal == "2532");
}
TEST_CASE("parse_decimal_lu"){
    auto ss = std::stringstream("2532lu");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(c.type == "unsigned long int");
    REQUIRE(c.literal == "2532");
}

//Tests exclusive to this stage (e.g. that the compiler fails on things that haven't been implemented yet)
#ifdef STAGE_4

#endif
