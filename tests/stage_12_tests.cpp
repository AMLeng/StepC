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


/*TEST_CASE("analyze struct decl in ptr"){
    auto ss = std::stringstream(
R"(
struct s{
    int a;
    double b;
} * n;
double f(struct s n){
    return n.a*n.b;
}
int main(){
    struct s x;
    n = &x;
    x.a = 4;
    x.b=0.5;
    return f(x);
}
)");
    lexer::Lexer l(ss);
    auto program_pointer =parse::construct_ast(l);
    program_pointer->analyze();
}*/

