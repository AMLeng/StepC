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
        if(type::is_signed_int(type::make_basic(source))){
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

value::Value* codegen_convert(type::BasicType target_type, value::Value* val, 
        std::ostream& output, context::Context& c){
    if(target_type == type::from_str("_Bool")){
        std::string command = std::visit(overloaded{
                [](type::IType){return "icmp ne";},
                [](type::FType){return "fcmp une";},
                }, val->get_type());

        AST::print_whitespace(c.depth(), output);
        auto new_tmp = c.new_temp(type::make_basic(type::IType::Bool));
        output << new_tmp->get_value() <<" = "<<command<<" "<<type::ir_type(val->get_type());
        output << std::visit(overloaded{
            [](type::IType){return " 0, ";},
            [](type::FType){return " 0.0, ";},
            }, val->get_type()) << val->get_value() <<std::endl;
        return new_tmp;
    }
    if(type::ir_type(target_type) == type::ir_type(val->get_type())){
        return std::move(val);
    }
    std::string command = std::visit([](auto t, auto s){
        return convert_command(t, s);
    }, target_type, val->get_type());

    AST::print_whitespace(c.depth(), output);
    auto new_tmp = c.new_temp(target_type);
    output << new_tmp->get_value() <<" = " << command <<" " << type::ir_type(val->get_type()) <<" ";
    output << val->get_value() << " to " << type::ir_type(target_type) << std::endl;
    return new_tmp;
}
} //namespace


value::Value* Program::codegen(std::ostream& output, context::Context& c){
    main_method->codegen(output, c);
    return nullptr;
}
value::Value* NullStmt::codegen(std::ostream& output, context::Context& c){
    //Do nothing
    return nullptr;
}

value::Value* IfStmt::codegen(std::ostream& output, context::Context& c){
    //To add
    return nullptr;
}
value::Value* CompoundStmt::codegen(std::ostream& output, context::Context& c){
    c.enter_scope();
    for(const auto& stmt : stmt_body){
        stmt->codegen(output, c);
    }
    c.exit_scope();
    return nullptr;
}
value::Value* FunctionDef::codegen(std::ostream& output, context::Context& c){
    assert(return_type == type::make_basic(type::IType::Int));
    AST::print_whitespace(c.depth(), output);
    output << "define "<<type::ir_type(return_type)<<" @" + name+"(){"<<std::endl;
    c.enter_function(return_type);
    function_body->codegen(output, c);
    c.exit_function();
    AST::print_whitespace(c.depth(), output);
    output << "}"<<std::endl;
    //Ultimately return value with
    //full function signature type
    //Once we add function argument/function types
    return nullptr;
}

value::Value* ReturnStmt::codegen(std::ostream& output, context::Context& c){
    auto return_value = return_expr->codegen(output, c);
    return_value = codegen_convert(c.return_type(),std::move(return_value), output, c);

    AST::print_whitespace(c.depth(), output);
    output << "ret "+type::ir_type(c.return_type())+" "+ return_value->get_value() << std::endl;
    return nullptr;
}

value::Value* Variable::codegen(std::ostream& output, context::Context& c){
    assert(this->analyzed && "This AST node has not had analysis run on it");
    auto var_value = c.get_value(variable_name);
    auto new_tmp = c.new_temp(var_value->get_type());
    AST::print_whitespace(c.depth(), output);
    output << new_tmp->get_value()<<" = load "<<type::ir_type(new_tmp->get_type());
    output << ", " <<type::ir_type(var_value->get_type())<<"* "<<var_value->get_value()<<std::endl;
    return new_tmp;
}

value::Value* VarDecl::codegen(std::ostream& output, context::Context& c){
    assert(this->analyzed && "This AST node has not had analysis run on it");
    auto variable = c.add_local(name, type);
    AST::print_whitespace(c.depth(), output);
    output << variable->get_value() <<" = alloca "<<type::ir_type(variable->get_type()) <<std::endl;
    if(this->assignment.has_value()){
        this->assignment.value()->codegen(output, c);
    }
    return nullptr;
}

value::Value* Constant::codegen(std::ostream& output, context::Context& c){
    assert(this->analyzed && "This AST node has not had analysis run on it");
    return c.add_literal(this->literal, this->type);
}

value::Value* UnaryOp::codegen(std::ostream& output, context::Context& c){
    assert(this->analyzed && "This AST node has not had analysis run on it");
    auto operand = arg->codegen(output, c);
    std::string t = type::ir_type(this->type);
    std::string command = "";
    value::Value* new_temp = nullptr;
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
            new_temp = c.new_temp(this->type);
            output << new_temp->get_value()<<" = "<<command<<" "<<t<<" 0, " <<operand->get_value() <<std::endl;
            return new_temp;
        case token::TokenType::BitwiseNot:
            operand =  codegen_convert(this->type, std::move(operand), output, c);
            AST::print_whitespace(c.depth(), output);
            new_temp = c.new_temp(this->type);
            output << new_temp->get_value()<<" = xor "<<t<<" -1, " <<operand->get_value() <<std::endl;
            return new_temp;
        case token::TokenType::Not:
        {
            assert(t == "i32");
            //icmp or fcmp
            command = std::visit(overloaded{
                [](type::IType){return "icmp eq";},
                [](type::FType){return "fcmp oeq";},
                }, operand->get_type());

            AST::print_whitespace(c.depth(), output);
            auto intermediate_bool = c.new_temp(type::make_basic(type::IType::Bool));
            output << intermediate_bool->get_value() <<" = "<<command<<" "<<type::ir_type(operand->get_type());
            output << std::visit(overloaded{
                [](type::IType){return " 0, ";},
                [](type::FType){return " 0.0, ";},
                }, operand->get_type()) << operand->get_value() <<std::endl;

            AST::print_whitespace(c.depth(), output);
            new_temp = c.new_temp(this->type);
            output << new_temp->get_value()<<" = zext i1 "<< intermediate_bool->get_value() <<" to "<<t<<std::endl;
        }
            return new_temp;
        default:
            assert(false && "Operator Not Implemented");
    }
}


value::Value* BinaryOp::codegen(std::ostream& output, context::Context& c){
    assert(this->analyzed && "This AST node has not had analysis run on it");
    auto right_register = this->right->codegen(output, c);
    //Operators that return an lvalue
    if(token::matches_type(tok, token::TokenType::Assign)){
        auto variable = dynamic_cast<ast::Variable*>(this->left.get());
        assert(variable && "Other lvalues not yet implemented");
        auto var_value = c.get_value(variable->variable_name);
        right_register = codegen_convert(this->type, std::move(right_register), output, c);
        switch(tok.type){
            case token::TokenType::Assign:
                AST::print_whitespace(c.depth(), output);
                output << "store "<<type::ir_type(right_register->get_type())<<" "<<right_register->get_value();
                output<<", "<<type::ir_type(this->type)<<"* "<<var_value->get_value()<<std::endl;
                break;
            default:
                assert(false && "Unknown binary assignment op during codegen");
        }
        return right_register;
    }
    //Operators that return an rvalue
    auto left_register = this->left->codegen(output, c);
    std::string t = type::ir_type(this->type);
    std::string command = "";
    switch(tok.type){
        case token::TokenType::Minus:
            left_register = codegen_convert(this->type, std::move(left_register), output, c);
            right_register = codegen_convert(this->type, std::move(right_register), output, c);
            command = std::visit(overloaded{
                [](type::IType){return "sub";},
                [](type::FType){return "fsub";},
                }, this->type);
            break;
        case token::TokenType::Plus:
            left_register = codegen_convert(this->type, std::move(left_register), output, c);
            right_register = codegen_convert(this->type, std::move(right_register), output, c);
            command = std::visit(overloaded{
                [](type::IType){return "add";},
                [](type::FType){return "fadd";},
                }, this->type);
            break;
        case token::TokenType::Mult:
            left_register = codegen_convert(this->type, std::move(left_register), output, c);
            right_register = codegen_convert(this->type, std::move(right_register), output, c);
            command = std::visit(overloaded{
                [](type::IType){return "mul";},
                [](type::FType){return "fmul";},
                }, this->type);
            break;
        case token::TokenType::Div:
            left_register = codegen_convert(this->type, std::move(left_register), output, c);
            right_register = codegen_convert(this->type, std::move(right_register), output, c);
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
    auto new_temp = c.new_temp(this->type);
    output << new_temp->get_value()<<" = "<<command<<" "<<t<<" " << left_register->get_value() <<", "<< right_register->get_value()<<std::endl;
    return new_temp;
}

} //namespace ast
