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
int main(){
int a[4];
int **b = &a;
}
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
TEST_CASE("Array in function def can have unknown size"){
    auto ss = std::stringstream(
R"(
int f(int a[]){
    return 4;
}
)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}

TEST_CASE("Function decl decay to ptr"){
    auto ss = std::stringstream(
R"(
int f(double *a);

int main(){
    double *a = 0;
    return f(a);
}
int f(double a[4]){
    return 4;
}
)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}

TEST_CASE("Function call decay to ptr"){
    auto ss = std::stringstream(
R"(
int f(double a[4]){
    return 4;
}
int g(int b(double * a)){
    double *x = 0;
    return b(x);
}
int main(){
    return g(f);
}
)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}

TEST_CASE("Constant expr evaluation"){
    auto ss = std::stringstream(
R"(
int b = (!~-1 || 1/0) && 14;
int main(){
    int a[(((3+1)*2/4)-(-3))%4 +(+4 & 3^1 |2)]; //Evaluates to 4
    return b;
}

)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("Switch statement const expr"){
    auto ss = std::stringstream(
R"(
int main(){
    int a = 4;
    switch(a){
        case 3*2-1:
            return 1;
        case 8/3:
            return 2;
        case 7 & 12:
            return 3;
    }
}

)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("Switch statement const expr duplicate error"){
    auto ss = std::stringstream(
R"(
int main(){
    int a = 4;
    switch(a){
        case 3*2-1:
            return 1;
        case 8/9:
            return 2;
        case 6%2:
            return 3;
    }
}

)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::STError);
}
TEST_CASE("String literal lexing"){
    auto ss = std::stringstream(
R"(
    "This is a string literal"
 "This is
        also a valid\n
            string literal!!!";
)");
    lexer::Lexer l(ss);
    l.get_token();
    l.get_token();
    REQUIRE(l.get_token().type == token::TokenType::Semicolon);
}
TEST_CASE("String literal lexing error"){
    auto ss = std::stringstream(
R"("This is not a valid string literal;

)");
    REQUIRE_THROWS_AS(lexer::Lexer(ss).get_token(),lexer_error::InvalidLiteral);
}

TEST_CASE("String literal parse"){
    auto ss = std::stringstream(
R"(
int printf();
int main(){
    char a[] = "This is a string literal";
    printf("This is another string literal");
}

)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}



