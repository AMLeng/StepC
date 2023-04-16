#include "lexer.h"
#include "parse.h"
#include "ast.h"

#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char* argv[]){
    std::unique_ptr<std::istream> input;
    if(argc == 1){
        input = std::make_unique<std::stringstream>(
R"(int main(){
    return ~!-4;
})");
    }else{
        input = std::make_unique<std::ifstream>(argv[1]);
    }
    assert(input != nullptr);
    lexer::Lexer l(*input);
    auto program_ast = parse::construct_ast(l);
    auto global_context = context::Context();
    program_ast->codegen(std::cout, global_context);
}
