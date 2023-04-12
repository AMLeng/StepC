#include "lexer.h"
#include "parse.h"
#include "ast.h"

#include <iostream>
#include <sstream>

int main(){
    auto input =std::stringstream(
R"(int main(){
    return 123;
})");
    lexer::Lexer l(input);
    auto program_ast = parse::construct_ast(l);
    program_ast->codegen(std::cout);
}
