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

TEST_CASE("parse short 1"){
    auto ss = std::stringstream(
R"(signed short)");
    lexer::Lexer l(ss);
    auto keywords = std::multiset<std::string>();
    while(l.peek_token().type == token::TokenType::Keyword){
        keywords.insert(l.get_token().value);
    }
    REQUIRE(type::from_str_multiset(keywords) == type::from_str("short int"));
}
TEST_CASE("parse unsigned long long"){
    auto ss = std::stringstream(
R"(long unsigned long int)");
    lexer::Lexer l(ss);
    auto keywords = std::multiset<std::string>();
    while(l.peek_token().type == token::TokenType::Keyword){
        keywords.insert(l.get_token().value);
    }
    REQUIRE(type::from_str_multiset(keywords) == type::from_str("unsigned long long int"));

}
TEST_CASE("parse unsigned long long 2"){
    auto ss = std::stringstream(
R"(unsigned long long)");
    lexer::Lexer l(ss);
    auto keywords = std::multiset<std::string>();
    while(l.peek_token().type == token::TokenType::Keyword){
        keywords.insert(l.get_token().value);
    }
    REQUIRE(type::from_str_multiset(keywords) == type::from_str("unsigned long long int"));

}
TEST_CASE("parse short 2"){
    auto ss = std::stringstream(
R"(int short)");
    lexer::Lexer l(ss);
    auto keywords = std::multiset<std::string>();
    while(l.peek_token().type == token::TokenType::Keyword){
        keywords.insert(l.get_token().value);
    }
    REQUIRE(type::from_str_multiset(keywords) == type::from_str("short int"));
}
TEST_CASE("parse var decl"){
    auto ss = std::stringstream(
R"(long int a;})");
    lexer::Lexer l(ss);
    auto a = parse::parse_var_decl(l);
    //a->pretty_print(0);
}
TEST_CASE("parse multiple stmt"){
    auto ss = std::stringstream(
R"(
    return 0;
    long unsigned a;
)");
    lexer::Lexer l(ss);
    auto ret = parse::parse_stmt(l);
    auto decl = parse::parse_stmt(l);
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

TEST_CASE("parse full program 1"){
    auto ss = std::stringstream(
R"(int main(){
    long unsigned a;
    3 + 4;
    return 12.3;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse full program 2"){
    auto ss = std::stringstream(
R"(int main(){
    long unsigned a;
    3 + 4;
    return a + 3;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse assignment"){
    auto ss = std::stringstream(
R"(
    b = (3 + a)*2;
)");
    lexer::Lexer l(ss);
    auto assign_ptr = parse::parse_binary_op(l,parse::parse_lvalue(l),0);
    //assign_ptr->pretty_print(0);
}
TEST_CASE("parse assignment in program"){
    auto ss = std::stringstream(
R"(int main(){
    b = (3 + a)*2;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("parse full program 3"){
    auto ss = std::stringstream(
R"(int main(){
    long unsigned a = 4;
    int b = (a + 2)*0.5;
    return ~b;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("multiple assign"){
    auto ss = std::stringstream(
R"(int main(){
    long unsigned a = 4;
    int b = a = 3;
    return b;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("variable redef"){
    auto ss = std::stringstream(
R"(int main(){
    long unsigned a = 4;
    int a = (a + 2)*0.5;
    return ~b;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::STError);
}
TEST_CASE("undefined variable"){
    auto ss = std::stringstream(
R"(int main(){
    return ~b;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::STError);
}
TEST_CASE("bad_lvalue"){
    auto ss = std::stringstream(
R"(int main(){
    int b;
    ~b = 3;
    return b;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
}
TEST_CASE("parse as bool"){
    auto ss = std::stringstream(
R"(
    _Bool b = 2;
)");
    lexer::Lexer l(ss);
    auto decl = parse::parse_var_decl(l);
    REQUIRE(decl->type == type::from_str("_Bool"));
    auto global_st = symbol::STable();
    decl->analyze(&global_st);
    REQUIRE(decl->assignment.value()->left->type == type::from_str("_Bool"));
}

//Tests exclusive to this stage (e.g. that the compiler fails on things that haven't been implemented yet)
//Tests which use structure that will be refactored later should not be here
//Since those tests should be updated during refactoring
#ifdef STAGE_5

#endif
