#include "lexer.h"
#include "parse.h"
#include "ast.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>

int main(int argc, char* argv[]){
    assert(argc == 2);
    auto file_name = std::string(argv[1]);
    auto input = std::ifstream(file_name);
    if(!input.is_open()){
        std::cout << "could not find file "<<file_name<<std::endl;
        return 1;
    }
    lexer::Lexer l(input);
    std::unique_ptr<ast::Program> program_ast = nullptr;
    try{
        program_ast = parse::construct_ast(l);
    }catch(std::exception& e){
        std::cout<<std::endl<<"Error compiling program "<<file_name<<std::endl;
        std::cout<<e.what()<<std::endl;
        return 1;
    }
    auto global_context = context::Context();

    if(file_name.substr(file_name.size() - 2, file_name.size()) != ".c"){
        std::cout << "unknown file extension "<<file_name<<std::endl;
        return 1;
    }
    auto program_name = file_name.substr(0,file_name.size() - 2);
    auto llc_command = "llc "+program_name+".ll";
    auto gpp_command = "g++ -o "+program_name+" "+program_name+".s";
    auto rm_llvm_ir = "rm "+program_name+".ll";
    auto rm_assembly = "rm "+program_name+".s";

    try{
        program_ast->analyze();
    }catch(std::exception& e){
        std::cout<<std::endl<<"Error compiling program "<<file_name<<std::endl;
        std::cout<<e.what()<<std::endl;
        return 1;
    }
    auto llvm_output = std::ofstream(program_name +".ll");
    //program_ast->pretty_print(0);
    program_ast->codegen(llvm_output, global_context); //Should output program_name .ll
    system(llc_command.c_str()); //Should output program_name .s
    system(rm_llvm_ir.c_str()); 
    system(gpp_command.c_str()); //Should output executable program_name
    system(rm_assembly.c_str());
}
