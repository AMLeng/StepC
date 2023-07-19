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
TEST_CASE("parse struct decl with var name"){
    auto ss = std::stringstream(
R"(
struct s{
    int a;
    double b;
} n;
int main(){
    struct a{
        int s;
        double t;
    } n;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("parse struct decl with var name no semicolon"){
    auto ss = std::stringstream(
R"(
struct s{
    int a;
    double b;
} n
int main(){
    struct a{
        int s;
        double t;
    } n;
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l),parse_error::ParseError);
}
TEST_CASE("parse struct decl no var name"){
    auto ss = std::stringstream(
R"(
struct s{
    int a;
    double b;
};
int main(){
    struct a{
        int s;
        double t;
    };
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("parse struct decl no var name no semicolon"){
    auto ss = std::stringstream(
R"(
struct s{
    int a;
    double b;
};
int main(){
    struct a{
        int s;
        double t;
    }
})");
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l),parse_error::ParseError);
}

TEST_CASE("struct return type local function decl error"){
    auto ss = std::stringstream(
R"(
int main(){
    struct s{int a;} x;
    struct s f(int y);
}
struct s f(int x){
    struct s a;
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::STError);
}
TEST_CASE("struct return type local function decl"){
    auto ss = std::stringstream(
R"(
struct s{int a;} x;
int main(){
    struct s f(int y);
}
struct s f(int x){
    struct s a;
    return a;
}
)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}

TEST_CASE("analyze struct decl in ptr"){
    auto ss = std::stringstream(
R"(
struct s{
    int a;
    double b;
} * n;
double f(struct s n){
    //return n.a*n.b;
    return 1;
}
int main(){
    struct s x;
    n = &x;
    //x.a = 4;
    //x.b=0.5;
    return f(x);
}
)");
    lexer::Lexer l(ss);
    auto program_pointer =parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("anon global structs"){
    auto ss = std::stringstream(
R"(
struct {
    int a;
}  n, p;
struct {
    int a;
}  m;
struct anon {
    int a;
}  l;
)");
    lexer::Lexer l(ss);
    auto program_pointer =parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("struct def in function return"){
    auto ss = std::stringstream(
R"(
struct  s{
    int a;
} f(int x);
int main(){
    struct s n;
    n = f(3);
}
)");
    lexer::Lexer l(ss);
    auto program_pointer =parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("nested struct def error"){
    auto ss = std::stringstream(
R"(
struct s{struct s a;} x;
)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::STError);
}
TEST_CASE("nested struct def"){
    auto ss = std::stringstream(
R"(
struct s{struct s * a;} x;
)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}


