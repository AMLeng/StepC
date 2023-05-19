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
        a += i;
    }
    return a;
})");
    lexer::Lexer l(ss);
    auto program_pointer = parse::construct_ast(l);
    program_pointer->analyze();
    //program_pointer->pretty_print(0);
}

//Tests exclusive to this stage (e.g. that the compiler fails on things that haven't been implemented yet)
//Tests which use structure that will be refactored later should not be here
//Since those tests should be updated during refactoring
#ifdef STAGE_8

#endif
