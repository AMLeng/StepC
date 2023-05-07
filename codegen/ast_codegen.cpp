#include "ast.h"
#include "sem_error.h"
#include "type.h"
#include <string>
#include <cassert>
namespace ast{

template <class... Ts>
struct overloaded : Ts...{
    using Ts::operator()...;
};

template<class...Ts> overloaded(Ts ...) -> overloaded<Ts...>;

namespace{
std::string convert_command(type::IType target, type::IType source){
    if(type::can_represent(target, source)){
        if(type::is_signed_int(type::make_basic(target))){
            return "sext";
        }else{
            return "zext";
        }
    }else{
        return "trunc";
    }
}
std::string convert_command(type::IType target, type::FType source){
    if(type::is_signed_int(type::make_basic(target))){
        //IF IT CAN'T FIT, POTENTIAL SOURCE OF Undefined Behavior!!!!!!!!!
        return "fptosi";
    }else{
        //IF IT CAN'T FIT, POTENTIAL SOURCE OF UB!!!!!!!!!
        return "fptoui";
    }
}

std::string convert_command(type::FType target, type::IType source){
    if(type::is_signed_int(type::make_basic(source))){
        return "sitofp";
    }else{
        return "uitofp";
    }
}
std::string convert_command(type::FType target, type::FType source){
    if(type::can_represent(target, source)){
        return "fpext";
    }else{
        return "fptrunc";
    }
}

std::unique_ptr<value::Value> codegen_convert(type::BasicType target_type, std::unique_ptr<value::Value> val, 
        std::ostream& output, context::Context& c){
    if(type::ir_type(target_type) == type::ir_type(val->get_type())){
        return std::move(val);
    }
    std::string command = std::visit([](auto t, auto s){
        return convert_command(t, s);
    }, target_type, val->get_type());

    AST::print_whitespace(c.depth(), output);
    output << c.new_temp()<<" = " << command <<" " << type::ir_type(val->get_type()) <<" ";
    output << val->get_value() << " to " << type::ir_type(target_type) << std::endl;
    return std::make_unique<value::Value>(c.prev_temp(0), target_type);
}
} //namespace


void AST::print_whitespace(int depth, std::ostream& output){
    for(int i=0; i<depth; i++){
        output << "  ";
    }
}

void Program::pretty_print(int depth){
    main_method->pretty_print(depth);
}

std::unique_ptr<value::Value> Program::codegen(std::ostream& output, context::Context& c){
    main_method->codegen(output, c);
    return nullptr;
}

void FunctionDef::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< type::to_string(return_type) << " FUNCTION "<<name <<":"<<std::endl;
    AST::print_whitespace(depth+1);
    std::cout<< "PARAMS: ()" << std::endl;
    AST::print_whitespace(depth+1);
    std::cout<< "BODY: " << std::endl;
    function_body -> pretty_print(depth+2);
}

std::unique_ptr<value::Value> FunctionDef::codegen(std::ostream& output, context::Context& c){
    assert(return_type == type::make_basic(type::IType::Int));
    AST::print_whitespace(c.depth(), output);
    output << "define "<<type::ir_type(return_type)<<" @" + name+"(){"<<std::endl;
    c.enter_function(return_type);
    function_body->codegen(output, c);
    c.exit_function();
    AST::print_whitespace(c.depth(), output);
    output << "}"<<std::endl;
    //Ultimately upgrade to full function signature type
    //Once we add function arguments
    return std::make_unique<value::Value>("@"+name, return_type); 
}

void ReturnStmt::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<< "RETURN:"<<std::endl;
    return_expr->pretty_print(depth+1);
}
std::unique_ptr<value::Value> ReturnStmt::codegen(std::ostream& output, context::Context& c){
    auto return_value = return_expr->codegen(output, c);
    return_value = codegen_convert(c.return_type(),std::move(return_value), output, c);

    AST::print_whitespace(c.depth(), output);
    output << "ret "+type::ir_type(c.return_type())+" "+ return_value->get_value() << std::endl;
    return nullptr;
}


void Constant::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<<"CONSTANT "<<literal<<" of type "<< to_string(type) <<std::endl;
}

std::unique_ptr<value::Value> Constant::codegen(std::ostream& output, context::Context& c){
    return std::make_unique<value::Value>(this->literal, this->type);
}

void UnaryOp::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<<"UNARY OP "<< tok.type <<" ON EXPR"<<std::endl;
    arg->pretty_print(depth+1);
}

std::unique_ptr<value::Value> UnaryOp::codegen(std::ostream& output, context::Context& c){
    auto operand = arg->codegen(output, c);
    std::string t = type::ir_type(this->type);
    std::string command = "";
    switch(tok.type){
        //Can't factor out since behavior for Not is not just one operation
        case token::TokenType::Plus:
            return codegen_convert(this->type, std::move(operand), output, c);
        case token::TokenType::Minus:
            operand =  codegen_convert(this->type, std::move(operand), output, c);
            //sub or fsub
            command = std::visit(overloaded{
                [](type::IType){return "sub";},
                [](type::FType){return "fsub";},
                }, operand->get_type());
            
            AST::print_whitespace(c.depth(), output);
            output << c.new_temp()<<" = "<<command<<" "<<t<<" 0, " <<operand->get_value() <<std::endl;
            return std::make_unique<value::Value>(c.prev_temp(0),this->type);
        case token::TokenType::BitwiseNot:
            operand =  codegen_convert(this->type, std::move(operand), output, c);
            AST::print_whitespace(c.depth(), output);
            output << c.new_temp()<<" = xor "<<t<<" -1, " <<operand->get_value() <<std::endl;
            return std::make_unique<value::Value>(c.prev_temp(0),this->type);
        case token::TokenType::Not:
            assert(t == "i32");
            //icmp or fcmp
            command = std::visit(overloaded{
                [](type::IType){return "icmp eq";},
                [](type::FType){return "fcmp oeq";},
                }, operand->get_type());

            AST::print_whitespace(c.depth(), output);
            output << c.new_temp()<<" = "<<command<<" "<<type::ir_type(operand->get_type());
            output << std::visit(overloaded{
                [](type::IType){return " 0, ";},
                [](type::FType){return " 0.0, ";},
                }, operand->get_type()) << operand->get_value() <<std::endl;

            AST::print_whitespace(c.depth(), output);
            output << c.new_temp()<<" = zext i1 "<< c.prev_temp(1) <<" to "<<t<<std::endl;
            return std::make_unique<value::Value>(c.prev_temp(0), this->type);
        default:
            assert(false && "Operator Not Implemented");
    }
}

void BinaryOp::pretty_print(int depth){
    AST::print_whitespace(depth);
    std::cout<<"BINARY OP "<< tok.type <<" WITH LEFT ARG"<<std::endl;
    left->pretty_print(depth+1);
    AST::print_whitespace(depth);
    std::cout<<"AND RIGHT ARG"<<std::endl;
    right->pretty_print(depth+1);
}

std::unique_ptr<value::Value> BinaryOp::codegen(std::ostream& output, context::Context& c){
    auto left_register = this->left->codegen(output, c);
    auto right_register = this->right->codegen(output, c);
    left_register = codegen_convert(this->type, std::move(left_register), output, c);
    right_register = codegen_convert(this->type, std::move(right_register), output, c);
    std::string t = type::ir_type(this->type);
    std::string command = "";
    switch(tok.type){
        case token::TokenType::Minus:
            command = std::visit(overloaded{
                [](type::IType){return "sub";},
                [](type::FType){return "fsub";},
                }, this->type);
            break;
        case token::TokenType::Plus:
            command = std::visit(overloaded{
                [](type::IType){return "add";},
                [](type::FType){return "fadd";},
                }, this->type);
            break;
        case token::TokenType::Mult:
            command = std::visit(overloaded{
                [](type::IType){return "mul";},
                [](type::FType){return "fmul";},
                }, this->type);
            break;
        case token::TokenType::Div:
            command = std::visit(overloaded{
                [](type::IType t){
                    if(type::is_signed_int(t)){return "sdiv";}
                    else{return "udiv";}},
                [](type::FType){return "fdiv";},
                }, this->type);
            break;
        default:
            assert(false && "Unknown binary op during codegen");
    }
    AST::print_whitespace(c.depth(), output);
    output << c.new_temp()<<" = "<<command<<" "<<t<<" " << left_register->get_value() <<", "<< right_register->get_value()<<std::endl;
    return std::make_unique<value::Value>(c.prev_temp(0),this->type);
}

} //namespace ast
