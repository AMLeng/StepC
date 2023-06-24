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


TEST_CASE("parse pointer decls"){
    auto ss = std::stringstream(
R"(
int main(){
    int* a;
    double** b;
    int (*c)(int, int);
    return 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("parse complex decl 1"){
    auto ss = std::stringstream(
R"(
int main(){
    void* (*a)(int *,double,...);
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("parse complex decl 2"){
    auto ss = std::stringstream(
R"(
int main(){
    int(b)(int*(*)(int));
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("parse complex decl 3"){
    auto ss = std::stringstream(
R"(
int main(){
    int(*(*c)(void))(int*(*)(int));
})");
    //Pointer to a function on (void) returning a pointer to
    //  "a function taking in an int*(*)(int) and returning an int"
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("parse complex decl 4"){
    auto ss = std::stringstream(
R"(
int main(){
    int**(*(**c)())(int);
})");
    //Pointer to pointer to a function returning a pointer to
    //  "a function taking in an int and returning an int**"
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
}
TEST_CASE("invalid decl 1"){
    auto ss = std::stringstream(
R"(
int main(){
    void* (a*)(int *,double,...);
})");
    //Identifier in incorrect spot relative to pointer *
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
}
TEST_CASE("invalid decl 2"){
    auto ss = std::stringstream(
R"(
int main(){
    int(int*(*)(int)) (*c)(void);
})");
    //Identifier must come before any function arg types
    lexer::Lexer l(ss);
    REQUIRE_THROWS_AS(parse::construct_ast(l), parse_error::ParseError);
}

TEST_CASE("address of and dereference"){
    auto ss = std::stringstream(
R"(
int main(){
    int a = 5;
    int *b = &a;
    return *b;
})");
    //Identifier must come before any function arg types
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}

TEST_CASE("invalid address of"){
    auto ss = std::stringstream(
R"(
int main(){
    &(3*4);
})");
    //Identifier must come before any function arg types
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
}
TEST_CASE("pointer cast "){
    auto ss = std::stringstream(
R"(
int main(){
    int a = 5;
    int * b = &a;
    void* c = b;
    return *b;
})");
    //Identifier must come before any function arg types
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("cannot return void"){
    auto ss = std::stringstream(
R"(
int main(){
    int a = 5;
    int * b = &a;
    void* c = b;
    return *c;
})");
    //Identifier must come before any function arg types
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
}

TEST_CASE("cannot deerefernce void"){
    auto ss = std::stringstream(
R"(
int main(){
    int a = 5;
    int * b = &a;
    void* c = b;
    *c;
})");
    //Identifier must come before any function arg types
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
}

TEST_CASE("change value with pointer"){
    auto ss = std::stringstream(
R"(
int main(){
    int a = 5;
    int * b = &a;
    *b = 6;
    return a;
})");
    //Identifier must come before any function arg types
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("assign nullptr"){
    auto ss = std::stringstream(
R"(
int main(){
    int* a = 0;
})");
    //Identifier must come before any function arg types
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("cannot assign non nullptr"){
    auto ss = std::stringstream(
R"(
int main(){
    int* a = 3;
})");
    //Identifier must come before any function arg types
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(),sem_error::TypeError);
}
TEST_CASE("cannot assign other int to ptr without cast"){
    auto ss = std::stringstream(
R"(
int main(){
    int* a = 3;
})");
    //Note that the result is 
    //"implementation defined
    //might not be correctly aligned
    //might not point to an entity of the referenced type
    //and might be a trap representation"
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
}
TEST_CASE("cannot assign incompatible ptr types without cast"){
    auto ss = std::stringstream(
R"(
int main(){
    int b = 3;
    int* a = &b;
    double* c = a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
}
TEST_CASE("can assign pointer to void* "){
    auto ss = std::stringstream(
R"(
int main(){
    int a =5;
    int* b = &a;
    void* c = b;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("can assign pointer from void* "){
    auto ss = std::stringstream(
R"(
int main(){
    int a =5;
    void* b = &a;
    int* c = b;
    return *c;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("can assign ptr to bool "){
    auto ss = std::stringstream(
R"(
int main(){
    int a =5;
    int* b = &a;
    _Bool c = b;
    return c;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
/* Have not yet implemented constant expressions
 * TEST_CASE("can assign complex 0 expression "){
    auto ss = std::stringstream(
R"(
int main(){
    int* a = (4-3)*12/15;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}*/
TEST_CASE("cannot convert ptr to int without cast"){
    auto ss = std::stringstream(
R"(
int main(){
    int a = 3;
    int*b = &a;
    return b;
})");
    //Note that even with a cast, the result is 
    //"implementation defined
    //might not be correctly aligned
    //might not point to an entity of the referenced type
    //and might be a trap representation"
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
}
TEST_CASE("cannot assign float to ptr"){
    auto ss = std::stringstream(
R"(
int main(){
    void* a = 3.4;
})");
    //Note that the result is 
    //"implementation defined
    //might not be correctly aligned
    //might not point to an entity of the referenced type
    //and might be a trap representation"
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
}
TEST_CASE("cannot assign ptr to float"){
    auto ss = std::stringstream(
R"(
int main(){
    float b = 24.0;
    void * a = &b;
    float c = a;
})");
    //Note that the result is 
    //"implementation defined
    //might not be correctly aligned
    //might not point to an entity of the referenced type
    //and might be a trap representation"
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
}
TEST_CASE("pointer int addition subtraction"){
    auto ss = std::stringstream(
R"(
int* array;
int main(){
    int size = 12;
    int index = 14;
    return *(array + index)+ *(size+ array - index);
}
)");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("pointer pointer subtraction"){
    auto ss = std::stringstream(
R"(
int* array;
int* array_end;
int main(){
    return array_end - array;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("pointer pointer relational"){
    auto ss = std::stringstream(
R"(
int main(){
    int a = 3;
    int* it1 = &a;;
    int* it2 = &a;;
    if(it1 < it2 || it2 < it1){
        return 0;
    }
    return 1;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("pointer pointer relational incompatible error"){
    auto ss = std::stringstream(
R"(
int* b;
double* c;
int main(){
    return b <= c;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(),sem_error::TypeError);
}
TEST_CASE("pointer void pointer relational error"){
    auto ss = std::stringstream(
R"(
int* b;
void* c;
int main(){
    return b >= c;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(), sem_error::TypeError);
}
TEST_CASE("pointer pointer equality"){
    auto ss = std::stringstream(
R"(
int* b;
int main(){
    int* a = 0;
    return b == a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("pointer pointer equality incompatible error"){
    auto ss = std::stringstream(
R"(
int* b;
double* c;
int main(){
    return b == c;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(),sem_error::TypeError);
}
TEST_CASE("pointer void pointer equality"){
    auto ss = std::stringstream(
R"(
int* b;
void* c;
int main(){
    return b != c;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("pointer null const equality"){
    auto ss = std::stringstream(
R"(
int* b;
int main(){
    return b == 0;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}
TEST_CASE("pointer nonnull const equality error"){
    auto ss = std::stringstream(
R"(
int* b;
int main(){
    return b == 5;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    REQUIRE_THROWS_AS(program_pointer->analyze(),sem_error::TypeError);
}
TEST_CASE("pass function pointer"){
    auto ss = std::stringstream(
R"(
int eval(int(*f)(int,int), int a, int b){
    return (*f)(a,b);
}
int plus(int a, int b){
    return a + b;
}
int main(){
    int(*p)(int, int) = &plus;
    return eval(p,3,7);
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
}


//Tests exclusive to this stage (e.g. that the compiler fails on things that haven't been implemented yet)
//Tests which use structure that will be refactored later should not be here
//Since those tests should be updated during refactoring
#ifdef STAGE_10

#endif
