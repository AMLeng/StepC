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


TEST_CASE("parse single function decl "){
    auto ss = std::stringstream(
R"(
int b(int p1, double p2);
int main(){
    return 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse function decl with param names"){
    auto ss = std::stringstream(
R"(
int a,b(int p1, double p2);
int main(){
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse function decl with param names 2"){
    auto ss = std::stringstream(
R"(
int a,b(int b, double a);
int main(){
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse function decl with same param names "){
    auto ss = std::stringstream(
R"(
int a,b(int a, double a);
int main(){
    return a;
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), sem_error::STError);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse function decl missing param names"){
    auto ss = std::stringstream(
R"(
int a,b(int , double );
int main(){
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse function decl missing param names 2"){
    auto ss = std::stringstream(
R"(
int a,b(int b, double);
int main(){
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("parse function decl void type"){
    auto ss = std::stringstream(
R"(
void b(int b, double);
int main(){
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    //program_pointer->pretty_print(0);
}
TEST_CASE("error no void variable"){
    auto ss = std::stringstream(
R"(int main(){
    void b;
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), sem_error::TypeError);
}
TEST_CASE("error no void argument name"){
    auto ss = std::stringstream(
R"(
int b(void a);
int main(){
    return b();
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), sem_error::TypeError);
}
TEST_CASE("error multiple function decl"){
    auto ss = std::stringstream(
R"(
void b(int);
void b(char);
int main(){
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::STError);
}
TEST_CASE("error multiple function decl 2"){
    auto ss = std::stringstream(
R"(
void b(int a);
int b(int a);
int main(){
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::STError);
}
TEST_CASE("function decl not at global scope"){
    auto ss = std::stringstream(
R"(
int main(){
    int b(int a);
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("parse global var"){
    auto ss = std::stringstream(
R"(
int a;
int main(){
    a = 3;
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("parse global var 2"){
    auto ss = std::stringstream(
R"(
int a = 4;
int main(){
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("parse global var multiple decl"){
    auto ss = std::stringstream(
R"(
int a;
int a = 3;
int main(){
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("error global var no multiple def"){
    auto ss = std::stringstream(
R"(
int a = 4;
int a = 3;
int main(){
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::STError);
}
TEST_CASE("error missing semicolon in decl"){
    auto ss = std::stringstream(
R"(
int a
int main(){
    return a;
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
}
TEST_CASE("error global var variable conflict"){
    auto ss = std::stringstream(
R"(
int a(int);
int a;
int main(){
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::STError);
}
TEST_CASE("error variable main"){
    auto ss = std::stringstream(
R"(
int main;
int main(){
    return 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::STError);
}
TEST_CASE("function call"){
    auto ss = std::stringstream(
R"(
int a(void);
int main(){
    return a();
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("function call convert params"){
    auto ss = std::stringstream(
R"(
float sum(float,float);
int main(){
    return sum(25, -3);
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("function call not defined"){
    auto ss = std::stringstream(
R"(
int putchar(int);
int main(){
    putchar(25);
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("error function call wrong params"){
    auto ss = std::stringstream(
R"(
int putchar(int);
int main(){
    putchar(25, 5);
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
}

TEST_CASE("error function call no decl"){
    auto ss = std::stringstream(
R"(
int main(){
    putchar(25);
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::STError);
}
TEST_CASE("error use void value"){
    auto ss = std::stringstream(
R"(
void a(int);
int main(){
    return a(25);
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
}
TEST_CASE("error function decl void variadic"){
    auto ss = std::stringstream(
R"(
int putchar(void,...);
int main(){
    putchar();
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
}
TEST_CASE("error function decl void multi arg"){
    auto ss = std::stringstream(
R"(
int putchar(void,int);
int main(){
    putchar(25, 5);
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
}
TEST_CASE("error function decl void multi arg 2"){
    auto ss = std::stringstream(
R"(
int putchar(void,void);
int main(){
    putchar();
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
}
TEST_CASE("empty return in void function"){
    auto ss = std::stringstream(
R"(
void a(){
    return;
}
)");
    lexer::Lexer l(ss);
    auto program = parse::construct_ast(l);
    program->analyze();
}
TEST_CASE("function def missing paren"){
    auto ss = std::stringstream(
R"(
int a{
    return;
}
)");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
}
TEST_CASE("main must return int"){
    auto ss = std::stringstream(
R"(
void main(){
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l)->analyze(), sem_error::TypeError);
}
TEST_CASE("variadic function decl no args"){
    auto ss = std::stringstream(
R"(
int a(...);
int main(){
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
}
TEST_CASE("variadic function decl one arg"){
    auto ss = std::stringstream(
R"(
int a(int b,...);
int main(){
    return a(3);
})");
    lexer::Lexer l(ss);
    parse::construct_ast(l)->analyze();
}
TEST_CASE("variadic function def one arg"){
    auto ss = std::stringstream(
R"(
int a(int b,...){
    return 3;
}
int main(){
    return a(4);
})");
    lexer::Lexer l(ss);
    parse::construct_ast(l)->analyze();
}

//Tests exclusive to this stage (e.g. that the compiler fails on things that haven't been implemented yet)
//Tests which use structure that will be refactored later should not be here
//Since those tests should be updated during refactoring
#ifdef STAGE_9

#endif
