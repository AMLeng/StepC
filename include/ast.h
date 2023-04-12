#include<memory>
#include<string>
#include<iostream>
#ifndef _AST_
#define _AST_
namespace ast{

class AST;
//List of node types
class Program;
class FunctionDef;
class ReturnStmt;
class Constant;

typedef std::string Type;

class AST{
public:
    virtual void pretty_print(int depth) = 0;
    static void print_whitespace(int depth);
    virtual void codegen(std::ostream& output) = 0;
};

class Program : public AST{
    std::unique_ptr<FunctionDef> main_method;
public:
    Program(std::unique_ptr<FunctionDef> main) : main_method(std::move(main)) {}
    void pretty_print(int depth) override;
    void codegen(std::ostream& output) override;
};

class FunctionDef : public AST{
    std::string name;
    Type return_type;
    std::unique_ptr<ReturnStmt> function_body;
public:
    FunctionDef(std::string name, Type ret_type, std::unique_ptr<ReturnStmt> ret) : name(name), return_type(ret_type), function_body(std::move(ret)) {}
    void pretty_print(int depth);
    void codegen(std::ostream& output) override;
};

class ReturnStmt : public AST{
    std::unique_ptr<Constant> return_value;
public:
    ReturnStmt(std::unique_ptr<Constant> ret_value) : return_value(std::move(ret_value)) {}
    void pretty_print(int depth);
    void codegen(std::ostream& output) override;
};

class Constant : public AST{
    std::string value;
    Type type;
public:
    Constant(std::string value, Type type) : value(value), type(type) {}
    void pretty_print(int depth);
    void codegen(std::ostream& output) override;
};

} //namespace ast
#endif
