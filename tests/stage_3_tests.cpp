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
    REQUIRE(l.get_token().type == token::TokenType::IntegerLiteral);
    REQUIRE(l.get_token().type == token::TokenType::Plus);
    REQUIRE(l.get_token().type == token::TokenType::IntegerLiteral);
}
TEST_CASE("recognizes_mult"){
    auto ss = std::stringstream("3*5");
    lexer::Lexer l(ss);
    REQUIRE(l.get_token().type == token::TokenType::IntegerLiteral);
    REQUIRE(l.get_token().type == token::TokenType::Mult);
    REQUIRE(l.get_token().type == token::TokenType::IntegerLiteral);
}
TEST_CASE("recognizes_logical_not"){
    auto ss = std::stringstream("5/4");
    lexer::Lexer l(ss);
    REQUIRE(l.get_token().type == token::TokenType::IntegerLiteral);
    REQUIRE(l.get_token().type == token::TokenType::Div);
    REQUIRE(l.get_token().type == token::TokenType::IntegerLiteral);
}

//Parser tests
TEST_CASE("parse_addition"){
    auto ss = std::stringstream(
R"(int main(){
    return 3+5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}

TEST_CASE("parse_mutiplication_by_negative"){
    auto ss = std::stringstream(
R"(int main(){
    return 3*-5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}

TEST_CASE("multiplication_higher_than_subtraction"){
    auto ss = std::stringstream(
R"(int main(){
    return 3*5-2;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("subtraction_left_associative"){
    auto ss = std::stringstream(
R"(int main(){
    return 3-5-2;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parentheses"){
    auto ss = std::stringstream(
R"(int main(){
    return 3*(5+2);
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("nested_parentheses"){
    auto ss = std::stringstream(
R"(int main(){
    return (3*(5+2)-4);
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("missing_l_paren"){
    auto ss = std::stringstream(
R"(int main(){
    return -4);
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
}
TEST_CASE("missing_r_paren"){
    auto ss = std::stringstream(
R"(int main(){
    return (-4*5;
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
}
TEST_CASE("parse_error_stage_three 1"){
    auto ss = std::stringstream(
R"(int main(){
    return /3;
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
}
TEST_CASE("parse_error_stage_three 2"){
    auto ss = std::stringstream(
R"(int main(){
    return 3-5-2+;
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
}
//Tests exclusive to this stage (e.g. that the compiler fails on things that haven't been implemented yet)
//Tests which use structure that will be refactored later should not be here
//Since those tests should be updated during refactoring
#ifdef STAGE_3

#endif
