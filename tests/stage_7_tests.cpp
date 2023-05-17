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


TEST_CASE("parse logical and"){
    auto ss = std::stringstream(
R"(int main(){
    long int a = 1;
    return 5 && a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse logical or"){
    auto ss = std::stringstream(
R"(int main(){
    long int a = 1;
    return 0 || a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse leq"){
    auto ss = std::stringstream(
R"(int main(){
    return 4 <= 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse less"){
    auto ss = std::stringstream(
R"(int main(){
    return 4 < 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse geq"){
    auto ss = std::stringstream(
R"(int main(){
    return 4 >= 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse greater"){
    auto ss = std::stringstream(
R"(int main(){
    return 4 > 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse not equal"){
    auto ss = std::stringstream(
R"(int main(){
    return 4 != 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse equal"){
    auto ss = std::stringstream(
R"(int main(){
    return 4 == 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse bitwise and"){
    auto ss = std::stringstream(
R"(int main(){
    return 4 & 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse bitwise or"){
    auto ss = std::stringstream(
R"(int main(){
    return 4 | 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse bitwise xor"){
    auto ss = std::stringstream(
R"(int main(){
    return 4 ^ 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse lshift"){
    auto ss = std::stringstream(
R"(int main(){
    return 4 << 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse rshift"){
    auto ss = std::stringstream(
R"(int main(){
    return 4 >> 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("sem bitwise and"){
    auto ss = std::stringstream(
R"(int main(){
    return 4.0 & 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
    //program_pointer->pretty_print(0);
}
TEST_CASE("sem bitwise or"){
    auto ss = std::stringstream(
R"(int main(){
    return 4 | 5.0;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
    //program_pointer->pretty_print(0);
}
TEST_CASE("sem bitwise xor"){
    auto ss = std::stringstream(
R"(int main(){
    return 4 ^ 5.3;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
    //program_pointer->pretty_print(0);
}
TEST_CASE("sem lshift"){
    auto ss = std::stringstream(
R"(int main(){
    return 4 << 5.0;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
}
TEST_CASE("sem rshift"){
    auto ss = std::stringstream(
R"(int main(){
    return 4.1 >> 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
    //program_pointer->pretty_print(0);
}
//Tests exclusive to this stage (e.g. that the compiler fails on things that haven't been implemented yet)
//Tests which use structure that will be refactored later should not be here
//Since those tests should be updated during refactoring
#ifdef STAGE_7

#endif
