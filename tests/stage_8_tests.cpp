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


TEST_CASE("parse for stmt"){
    auto ss = std::stringstream(
R"(int main(){
    int a = 0;
    for(int i=0; i<5; i++){
        a += i;
    }
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse for stmt empty"){
    auto ss = std::stringstream(
R"(int main(){
    int a = 0;
    for(;;){
        a += 1;
    }
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse do stmt"){
    auto ss = std::stringstream(
R"(int main(){
    int a = 0;
    do
        a += 1;
    while(a > 0);
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse do stmt error"){
    auto ss = std::stringstream(
R"(int main(){
    int a = 0;
    do a -= 1; while(a > 0)
    return a;
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse do decl not stmt error"){
    auto ss = std::stringstream(
R"(int main(){
    int a = 0;
    do int a = 100; while(a < 0)
    return a;
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse while stmt"){
    auto ss = std::stringstream(
R"(int main(){
    int a = 0;
    while(a < 4){
        a += 1;
    }
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse while error"){
    auto ss = std::stringstream(
R"(int main(){
    int a = 0;
    while(){
        a += 1;
    }
    return a;
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse continue "){
    auto ss = std::stringstream(
R"(int main(){
    int a = 0;
    while(a < 20){
        a += 1;
    continue;
    }
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse break "){
    auto ss = std::stringstream(
R"(int main(){
    int a = 0;
    while(a < 20){
        a += 1;
    break;
    }
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
    //program_pointer->pretty_print(0);
}
TEST_CASE("break outside loop error"){
    auto ss = std::stringstream(
R"(int main(){
    int a = 0;
    break;
    while(a < 20){
        a += 1;
    }
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::FlowError);
    //program_pointer->pretty_print(0);
}
TEST_CASE("continue outside loop error"){
    auto ss = std::stringstream(
R"(int main(){
    int a = 0;
    continue;
    while(a < 20){
        a += 1;
    }
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::FlowError);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse labels"){
    auto ss = std::stringstream(
R"(int main(){
    int a = 0;
    bob:
    while(a < 20){
        a += 1;
    }
    joe:
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
    //program_pointer->pretty_print(0);
}
TEST_CASE("sem error duplicate label"){
    auto ss = std::stringstream(
R"(int main(){
    int a = 0;
    bob:
    while(a < 20){
        a += 1;
    }
    bob:
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::STError);
    //program_pointer->pretty_print(0);
}
TEST_CASE("sem error missing label"){
    auto ss = std::stringstream(
R"(int main(){
    int a = 0;
    goto bob;
    while(a < 20){
        a += 1;
    }
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::STError);
    //program_pointer->pretty_print(0);
}

//Tests exclusive to this stage (e.g. that the compiler fails on things that haven't been implemented yet)
//Tests which use structure that will be refactored later should not be here
//Since those tests should be updated during refactoring
#ifdef STAGE_8

#endif
