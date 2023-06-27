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
TEST_CASE("parse array decl"){
    auto ss = std::stringstream(
R"(
int main(){
    int a[4];
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("array function decl"){
    auto ss = std::stringstream(
R"(
int sum(int a[], int size);
)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("array function def"){
    auto ss = std::stringstream(
R"(
int sum(int a[], int size){
    int s = 0;
    for(int i=0; i<size; i++){
        s += a[i];
    }
}
)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("invalid array decl"){
    auto ss = std::stringstream(
R"(
int main(){
    int a[];
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(),sem_error::TypeError);
}
TEST_CASE("initialization list for scalar"){
    auto ss = std::stringstream(
R"(
int main(){
    int a = {4};
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("empty scalar initialization error"){
    auto ss = std::stringstream(
R"(
int main(){
    int a = {};
})");
    lexer::Lexer l(ss);
    auto p = parse::construct_ast(l);
    REQUIRE_THROWS_AS(p->analyze(), sem_error::TypeError);
}
TEST_CASE("parse array init decl"){
    auto ss = std::stringstream(
R"(
int main(){
    int a[4] = {1,2,3,4};
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("parse array init decl empty"){
    auto ss = std::stringstream(
R"(
int main(){
    int a[4] = {};
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("parse unknown size array init decl empty"){
    auto ss = std::stringstream(
R"(
int main(){
    int a[] = {};
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("parse array init decl not enough"){
    auto ss = std::stringstream(
R"(
int main(){
    int a[4] = {1,2,3};
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("parse array infer size init decl"){
    auto ss = std::stringstream(
R"(
int main(){
    int a[] = {1,2,3,4};
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("parse array infer size init decl string"){
    auto ss = std::stringstream(
R"(
int main(){
    char a[] = "this";
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("Pointer to array vs pointer to pointer error"){
    auto ss = std::stringstream(
R"(
int a[4];
int **b = &a;
)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
}
TEST_CASE("Can't increment array error"){
    auto ss = std::stringstream(
R"(
int main(){
    int a[4];
    a++;
}
)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
}




