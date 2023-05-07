#include "catch2/catch.hpp"
#include "lexer.h"
#include "type.h"
#include "lexer_error.h"
#include "parse_error.h"
#include "sem_error.h"
#include "parse.h"
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>
#include <string>


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

//Type conversion/promotion tests
TEST_CASE("promote_char_to_int"){
    auto i = type::from_str("int");
    auto c = type::from_str("char");
    REQUIRE(i == type::integer_promotions(c));
}

TEST_CASE("promote_schar_to_int"){
    auto i = type::from_str("int");
    auto c = type::from_str("signed char");
    REQUIRE(i == type::integer_promotions(c));
}
TEST_CASE("promote_ushort_to_int"){
    auto i = type::from_str("int");
    auto s = type::from_str("unsigned short int");
    REQUIRE(i == type::integer_promotions(s));
}
TEST_CASE("promote_uint_to_uint"){
    auto i = type::from_str("unsigned int");
    auto s = type::from_str("unsigned int");
    REQUIRE(i == type::integer_promotions(s));
}
//Test the arithmetic conversions for the types we've implemented
TEST_CASE("usual_arith char char"){
    auto a = type::from_str("char");
    auto b = type::from_str("char");
    auto result = type::from_str("int");
    REQUIRE(result == type::usual_arithmetic_conversions(a,b));
}
TEST_CASE("usual_arith char double"){
    auto a = type::from_str("char");
    auto b = type::from_str("double");
    auto result = type::from_str("double");
    REQUIRE(result == type::usual_arithmetic_conversions(a,b));
}
TEST_CASE("usual_arith char long"){
    auto a = type::from_str("char");
    auto b = type::from_str("long int");
    auto result = type::from_str("long int");
    REQUIRE(result == type::usual_arithmetic_conversions(a,b));
}
TEST_CASE("usual_arith long short"){
    auto a = type::from_str("long int");
    auto b = type::from_str("short int");
    auto result = type::from_str("long int");
    REQUIRE(result == type::usual_arithmetic_conversions(a,b));
}
TEST_CASE("usual_arith short ulong"){
    auto a = type::from_str("short int");
    auto b = type::from_str("unsigned long int");
    auto result = type::from_str("unsigned long int");
    REQUIRE(result == type::usual_arithmetic_conversions(a,b));
}
TEST_CASE("usual_arith ushort long"){
    auto a = type::from_str("unsigned short int");
    auto b = type::from_str("long int");
    auto result = type::from_str("long int");
    REQUIRE(result == type::usual_arithmetic_conversions(a,b));
}
TEST_CASE("usual_arith ull ld"){
    auto a = type::from_str("unsigned long long int");
    auto b = type::from_str("long double");
    auto result = type::from_str("long double");
    REQUIRE(result == type::usual_arithmetic_conversions(a,b));
}
TEST_CASE("usual_arith d ll"){
    auto a = type::from_str("double");
    auto b = type::from_str("long long int");
    auto result = type::from_str("double");
    REQUIRE(result == type::usual_arithmetic_conversions(a,b));
}
TEST_CASE("usual_arith ull f"){
    auto a = type::from_str("unsigned long long int");
    auto b = type::from_str("float");
    auto result = type::from_str("float");
    REQUIRE(result == type::usual_arithmetic_conversions(a,b));
}
TEST_CASE("usual_arith us s"){
    auto a = type::from_str("unsigned short int");
    auto b = type::from_str("short int");
    auto result = type::from_str("int");
    REQUIRE(result == type::usual_arithmetic_conversions(a,b));
}
TEST_CASE("usual_arith us l"){
    auto a = type::from_str("unsigned short int");
    auto b = type::from_str("long int");
    auto result = type::from_str("long int");
    REQUIRE(result == type::usual_arithmetic_conversions(a,b));
}
TEST_CASE("usual_arith l ll"){
    auto a = type::from_str("long int");
    auto b = type::from_str("long long int");
    auto result = type::from_str("long long int");
    REQUIRE(result == type::usual_arithmetic_conversions(a,b));
}
TEST_CASE("usual_arith ull l"){
    auto a = type::from_str("unsigned long long int");
    auto b = type::from_str("long int");
    auto result = type::from_str("unsigned long long int");
    REQUIRE(result == type::usual_arithmetic_conversions(a,b));
}
TEST_CASE("usual_arith s ul"){
    auto a = type::from_str("short int");
    auto b = type::from_str("unsigned long int");
    auto result = type::from_str("unsigned long int");
    REQUIRE(result == type::usual_arithmetic_conversions(a,b));
}
TEST_CASE("usual_arith ui ll"){
    auto a = type::from_str("unsigned int");
    auto b = type::from_str("long long int");
    auto result = type::from_str("long long int");
    REQUIRE(result == type::usual_arithmetic_conversions(a,b));
}
TEST_CASE("usual_arith l ui"){
    auto a = type::from_str("long int");
    auto b = type::from_str("unsigned int");
    auto result = type::from_str("long int");
    REQUIRE(result == type::usual_arithmetic_conversions(a,b));
}
TEST_CASE("usual_arith ll ul"){
    auto a = type::from_str("long long int");
    auto b = type::from_str("unsigned long int");
    auto result = type::from_str("unsigned long long int");
    REQUIRE(result == type::usual_arithmetic_conversions(a,b));
}
TEST_CASE("usual_arith ul ll"){
    auto a = type::from_str("unsigned long int");
    auto b = type::from_str("long long int");
    auto result = type::from_str("unsigned long long int");
    REQUIRE(result == type::usual_arithmetic_conversions(a,b));
}

//End type conversion tests

//Parser tests, type checking
TEST_CASE("parse_float_dec"){
    auto ss = std::stringstream(
R"(1.1f)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(std::get<type::FType>(c.type) ==  type::FType::Float);
    REQUIRE(c.literal == "0X3FF19999A0000000");
}
TEST_CASE("parse_double_dec"){
    auto ss = std::stringstream(
R"(1.1)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(std::get<type::FType>(c.type) ==  type::FType::Double);
    REQUIRE(c.literal == "0X3FF199999999999A");
}
TEST_CASE("parse_hex_float"){
    auto ss = std::stringstream(
R"(0x1.1999999999999999Ap-64f)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(std::get<type::FType>(c.type) ==  type::FType::Float);
    REQUIRE(c.literal == "0X3BF19999A0000000");
}
TEST_CASE("parse_hex_float_normal"){
    auto ss = std::stringstream(
R"(0x1.19999Ap-64f)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(std::get<type::FType>(c.type) ==  type::FType::Float);
    REQUIRE(c.literal == "0X3BF19999A0000000");
}
TEST_CASE("parse_hex_float_barely_subnormal"){
    auto ss = std::stringstream(
R"(0x1.19999Ap-127F)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(std::get<type::FType>(c.type) ==  type::FType::Float);
    REQUIRE(c.literal == "0X3801999980000000");
}
TEST_CASE("parse_hex_float_subnormal_2"){
    auto ss = std::stringstream(
R"(0x1.19999Ap-129f)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(std::get<type::FType>(c.type) ==  type::FType::Float);
    REQUIRE(c.literal == "0X37E1999A00000000");
}
TEST_CASE("parse_hex_double"){
    auto ss = std::stringstream(
R"(0x1.1999999999999999Ap-64)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(std::get<type::FType>(c.type) ==  type::FType::Double);
    REQUIRE(c.literal == "0X3BF199999999999A");
}
TEST_CASE("parse_hex_long_double"){
    auto ss = std::stringstream(
R"(0x1.19999999999999999Ap-64l)");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(std::get<type::FType>(c.type) ==  type::FType::LDouble);
    REQUIRE(c.literal == "0X3BF199999999999A");
}
TEST_CASE("parse_decimal_U"){
    auto ss = std::stringstream("2532U");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(std::get<type::IType>(c.type) ==  type::IType::UInt);
    REQUIRE(c.literal == "2532");
}
TEST_CASE("parse_decimal_u"){
    auto ss = std::stringstream("2532u");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(std::get<type::IType>(c.type) ==  type::IType::UInt);
    REQUIRE(c.literal == "2532");
}
TEST_CASE("parse_decimal_ull"){
    auto ss = std::stringstream("2532ull");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(std::get<type::IType>(c.type) ==  type::IType::ULLong);
    REQUIRE(c.literal == "2532");
}
TEST_CASE("parse_decimal_LLu"){
    auto ss = std::stringstream("2532LLu");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(std::get<type::IType>(c.type) ==  type::IType::ULLong);
    REQUIRE(c.literal == "2532");
}
TEST_CASE("parse_decimal_lu"){
    auto ss = std::stringstream("2532lu");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(std::get<type::IType>(c.type) ==  type::IType::ULong);
    REQUIRE(c.literal == "2532");
}
TEST_CASE("parse_dec_int"){
    auto ss = std::stringstream("25");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(std::get<type::IType>(c.type) ==  type::IType::Int);
    REQUIRE(c.literal == "25");
}

TEST_CASE("parse_unary_plus"){
    auto ss = std::stringstream(
R"(int main(){
    return 4+(+3);
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}

//Weirder parsing cases where we need to promote
//THESE CAN BE TARGET DEPENDENT!!!!!!!!
TEST_CASE("parse_unsigned_hex_int_target_dependent"){
    auto ss = std::stringstream("0xAFFFFFFF");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(std::get<type::IType>(c.type) ==  type::IType::UInt);
    REQUIRE(c.literal == "2952790015");
}
TEST_CASE("parse_too_large_dec_int_target_dependent"){
    auto ss = std::stringstream("2952790015");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(std::get<type::IType>(c.type) ==  type::IType::Long);
    REQUIRE(c.literal == "2952790015");
}
TEST_CASE("parse_unsigned_hex_long_target_dependent"){
    auto ss = std::stringstream("01177777777777777777777");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    auto c = ast::Constant(t);
    REQUIRE(std::get<type::IType>(c.type) ==  type::IType::ULong);
    REQUIRE(c.literal == "11529215046068469759");
}
TEST_CASE("parse_too_large_dec_target_dependent"){
    auto ss = std::stringstream("49517601571415210995964968959");
    lexer::Lexer l(ss);
    auto t = l.get_token();
    REQUIRE_THROWS_AS(ast::Constant(t),sem_error::TypeError);
}

//Full program parses
TEST_CASE("parse double"){
    auto ss = std::stringstream(
R"(int main(){
    return 3.0;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse not double"){
    auto ss = std::stringstream(
R"(int main(){
    return !3.1375;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse minus float"){
    auto ss = std::stringstream(
R"(int main(){
    return -3.4f;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse bitwisenot ullong unspecified behavior"){
    auto ss = std::stringstream(
R"(int main(){
    return ~1ull;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}

//Tests exclusive to this stage (e.g. that the compiler fails on things that haven't been implemented yet)
#ifdef STAGE_4

#endif
