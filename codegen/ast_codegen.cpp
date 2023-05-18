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
value::Value* make_command(type::BasicType t, std::string command, value::Value* left, value::Value* right, 
    std::ostream& output, context::Context& c){
    AST::print_whitespace(c.depth(), output);
    auto new_temp = c.new_temp(t);
    output << new_temp->get_value()<<" = "<<command<<" "<<type::ir_type(left->get_type())<<" " << left->get_value() <<", "<< right->get_value()<<std::endl;
    return new_temp;
}

value::Value* make_load(value::Value* local, std::ostream& output, context::Context& c){
    auto result = c.new_temp(local->get_type());
    AST::print_whitespace(c.depth(), output);
    output << result->get_value()<<" = load "<<type::ir_type(local->get_type());
    output << ", " <<type::ir_type(local->get_type())<<"* "<<local->get_value()<<std::endl;
    return result;
}
value::Value* make_tmp_reg(type::BasicType t, std::ostream& output, context::Context& c){
    auto new_tmp = c.new_temp(t);
    AST::print_whitespace(c.depth(), output);
    output << new_tmp->get_value() <<" = alloca "<<type::ir_type(new_tmp->get_type()) <<std::endl;
    return new_tmp;
}
void make_store(value::Value* val, value::Value* reg, std::ostream& output, context::Context& c){
    AST::print_whitespace(c.depth(), output);
    output << "store "<<type::ir_type(val->get_type())<<" "<<val->get_value();
    output<<", "<<type::ir_type(reg->get_type())<<"* "<<reg->get_value()<<std::endl;
}


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

value::Value* assignment_codegen(const ast::BinaryOp* node, std::ostream& output, context::Context& c){
    auto right_register = node->right->codegen(output, c);
    right_register = codegen_convert(node->new_right_type, std::move(right_register), output, c);
    auto ast_variable = dynamic_cast<const ast::Variable*>(node->left.get());
    assert(ast_variable && "Other lvalues not yet implemented");
    auto var_value = c.get_value(ast_variable->variable_name);
    auto loaded_value = make_load(var_value, output, c);
    loaded_value = codegen_convert(node->new_left_type,loaded_value, output, c);

    value::Value* result = nullptr;
    switch(node->tok.type){
        case token::TokenType::PlusAssign:
            result = make_command(node->new_left_type,std::visit(overloaded{
                [](type::IType){return "add";},
                [](type::FType){return "fadd";},
                }, node->type) , loaded_value,right_register,output,c);
            break;
        case token::TokenType::MinusAssign:
            result = make_command(node->new_left_type, std::visit(overloaded{
                [](type::IType){return "sub";},
                [](type::FType){return "fsub";},
                }, node->type), loaded_value,right_register,output,c);
            break;
        case token::TokenType::DivAssign:
            result = make_command(node->new_left_type,std::visit(overloaded{
                [](type::IType t){
                    if(type::is_signed_int(t)){return "sdiv";}
                    else{return "udiv";}},
                [](type::FType){return "fdiv";},
                }, node->type) , loaded_value,right_register,output,c);
            break;
        case token::TokenType::MultAssign:
            result = make_command(node->new_left_type,std::visit(overloaded{
                [](type::IType){return "mul";},
                [](type::FType){return "fmul";},
                }, node->type) , loaded_value,right_register,output,c);
            break;
        case token::TokenType::ModAssign:
            result = make_command(node->new_left_type,std::visit(overloaded{
                [](type::IType t){
                    if(type::is_signed_int(t)){return "srem";}
                    else{return "urem";}},
                [](type::FType){
                    assert(false && "C does not allow mod to take floating point arguments");
                    return "frem";},
                }, node->type) , loaded_value,right_register,output,c);
            break;
        case token::TokenType::LSAssign:
            right_register = codegen_convert(node->new_left_type, right_register, output, c);
            result = make_command(node->new_left_type,"shl", loaded_value,right_register,output,c);
            break;
        case token::TokenType::RSAssign:
            //LLVM IR requires both arguments to the shift to be the same integer type
            right_register = codegen_convert(node->new_left_type, right_register, output, c);
            //C standard says that if the left operand is signed and negative, then UB
            //So it doesn't matter if we lshr or ashr
            result = make_command(node->new_left_type,"lshr", loaded_value,right_register,output,c);
            break;
        case token::TokenType::BAAssign:
            result = make_command(node->new_left_type,"and", loaded_value,right_register,output,c);
            break;
        case token::TokenType::BOAssign:
            result = make_command(node->new_left_type,"or", loaded_value,right_register,output,c);
            break;
        case token::TokenType::BXAssign:
            result = make_command(node->new_left_type,"xor", loaded_value,right_register,output,c);
            break;
        case token::TokenType::Assign:
            result = right_register;
            break;
        default:
            assert(false && "Unknown binary assignment op during codegen");
    }
    result = codegen_convert(var_value->get_type(), result, output, c);
    make_store(result, var_value, output, c);
    return result;
}
value::Value* short_circuit_codegen(const ast::BinaryOp* node, std::ostream& output, context::Context& c){
    int instruction_number = c.new_local_name(); 
    std::string no_sc_label = "logical_op_no_sc."+std::to_string(instruction_number);
    std::string end_label = "logical_op_end."+std::to_string(instruction_number);

    auto left_register = node->left->codegen(output, c);
    left_register = codegen_convert(type::make_basic(type::IType::Bool),left_register, output, c);

    auto new_tmp = make_tmp_reg(type::make_basic(type::IType::Bool), output, c);
    make_store(left_register, new_tmp, output, c);

    switch(node->tok.type){
        case token::TokenType::And:
            c.change_block(no_sc_label, output, 
                std::make_unique<basicblock::Cond_BR>(left_register, no_sc_label,end_label));
            break;
        case token::TokenType::Or:
            c.change_block(no_sc_label, output, 
                std::make_unique<basicblock::Cond_BR>(left_register, end_label,no_sc_label));
            break;
        default:
            assert(false && "Unknown binary assignment op during codegen");
    }
    auto right_register = node->right->codegen(output, c);
    right_register = codegen_convert(type::make_basic(type::IType::Bool),right_register, output, c);
    value::Value* no_sc_result = nullptr;
    switch(node->tok.type){
        case token::TokenType::And:
            no_sc_result = make_command(type::from_str("_Bool"),"and",left_register,right_register,output,c);
            break;
        case token::TokenType::Or:
            no_sc_result = make_command(type::from_str("_Bool"),"or",left_register,right_register,output,c);
            break;
        default:
            assert(false && "Unknown binary assignment op during codegen");
    }
    make_store(no_sc_result, new_tmp, output, c);
    c.change_block(end_label,output,std::make_unique<basicblock::UCond_BR>(end_label));

    auto result = make_load(new_tmp, output, c);
    result = codegen_convert(node->type, result, output, c);
    return result;
}
value::Value* other_bin_op_codegen(const ast::BinaryOp* node, std::ostream& output, context::Context& c){
    auto left_register = node->left->codegen(output, c);
    left_register = codegen_convert(node->new_left_type, std::move(left_register), output, c);
    auto right_register = node->right->codegen(output, c);
    right_register = codegen_convert(node->new_right_type, std::move(right_register), output, c);
    value::Value* result = nullptr;
    switch(node->tok.type){
        case token::TokenType::Minus:
            result = make_command(node->type, std::visit(overloaded{
                [](type::IType){return "sub";},
                [](type::FType){return "fsub";},
                }, node->type), left_register,right_register,output,c);
            break;
        case token::TokenType::Plus:
            result = make_command(node->type,std::visit(overloaded{
                [](type::IType){return "add";},
                [](type::FType){return "fadd";},
                }, node->type) , left_register,right_register,output,c);
            break;
        case token::TokenType::Mult:
            result = make_command(node->type,std::visit(overloaded{
                [](type::IType){return "mul";},
                [](type::FType){return "fmul";},
                }, node->type) , left_register,right_register,output,c);
            break;
        case token::TokenType::Div:
            result = make_command(node->type,std::visit(overloaded{
                [](type::IType t){
                    if(type::is_signed_int(t)){return "sdiv";}
                    else{return "udiv";}},
                [](type::FType){return "fdiv";},
                }, node->type) , left_register,right_register,output,c);
            break;
        case token::TokenType::Mod:
            result = make_command(node->type,std::visit(overloaded{
                [](type::IType t){
                    if(type::is_signed_int(t)){return "srem";}
                    else{return "urem";}},
                [](type::FType){
                    assert(false && "C does not allow mod to take floating point arguments");
                    return "frem";},
                }, node->type) , left_register,right_register,output,c);
            break;
        case token::TokenType::Equal:
            result = make_command(type::from_str("_Bool"),std::visit(overloaded{
                [](type::IType){return "icmp eq";},
                [](type::FType){return "fcmp oeq";},
                }, node->new_left_type) , left_register,right_register,output,c);
            break;
        case token::TokenType::NEqual:
            result = make_command(type::from_str("_Bool"),std::visit(overloaded{
                [](type::IType){return "icmp ne";},
                [](type::FType){return "fcmp one";},
                }, node->new_left_type) , left_register,right_register,output,c);
            break;
        case token::TokenType::Less:
            result = make_command(type::from_str("_Bool"),std::visit(overloaded{
                [](type::IType t){
                    if(type::is_signed_int(t)){return "icmp slt";}
                    else{return "icmp ult";}},
                [](type::FType){return "fcmp olt";},
                }, node->new_left_type) , left_register,right_register,output,c);
            break;
        case token::TokenType::Greater:
            result = make_command(type::from_str("_Bool"),std::visit(overloaded{
                [](type::IType t){
                    if(type::is_signed_int(t)){return "icmp sgt";}
                    else{return "icmp ugt";}},
                [](type::FType){return "fcmp ogt";},
                }, node->new_left_type) , left_register,right_register,output,c);
            break;
        case token::TokenType::LEq:
            result = make_command(type::from_str("_Bool"),std::visit(overloaded{
                [](type::IType t){
                    if(type::is_signed_int(t)){return "icmp sle";}
                    else{return "icmp ule";}},
                [](type::FType){return "fcmp ole";},
                }, node->new_left_type) , left_register,right_register,output,c);
            break;
        case token::TokenType::GEq:
            result = make_command(type::from_str("_Bool"),std::visit(overloaded{
                [](type::IType t){
                    if(type::is_signed_int(t)){return "icmp sge";}
                    else{return "icmp uge";}},
                [](type::FType){return "fcmp oge";},
                }, node->new_left_type) , left_register,right_register,output,c);
            break;
        case token::TokenType::LShift:
            //LLVM IR requires both arguments to the shift to be the same integer type
            right_register = codegen_convert(node->new_left_type, right_register, output, c);
            result = make_command(node->type,"shl", left_register,right_register,output,c);
            break;
        case token::TokenType::RShift:
            //LLVM IR requires both arguments to the shift to be the same integer type
            right_register = codegen_convert(node->new_left_type, right_register, output, c);
            //C standard says that if the left operand is signed and negative, then UB
            //So it doesn't matter if we lshr or ashr
            result = make_command(node->type,"lshr", left_register,right_register,output,c);
            break;
        case token::TokenType::BitwiseAnd:
            result = make_command(node->type,"and", left_register,right_register,output,c);
            break;
        case token::TokenType::BitwiseOr:
            result = make_command(node->type,"or", left_register,right_register,output,c);
            break;
        case token::TokenType::BitwiseXor:
            result = make_command(node->type,"xor", left_register,right_register,output,c);
            break;
        case token::TokenType::Comma:
            result = right_register;
            break;
        default:
            assert(false && "Unknown binary op during codegen");
    }
    return codegen_convert(node->type, result, output, c);
}
} //namespace


value::Value* Program::codegen(std::ostream& output, context::Context& c)const {
    main_method->codegen(output, c);
    return nullptr;
}
value::Value* NullStmt::codegen(std::ostream& output, context::Context& c)const {
    //Do nothing
    return nullptr;
}
value::Value* Conditional::codegen(std::ostream& output, context::Context& c)const {
    assert(this->analyzed && "This AST node has not had analysis run on it");
    auto condition = cond->codegen(output, c);
    condition = codegen_convert(type::make_basic(type::IType::Bool),condition, output, c);
    auto new_tmp = make_tmp_reg(this->type, output, c);

    int instruction_number = c.new_local_name(); 
    std::string true_label = "condtrue."+std::to_string(instruction_number);
    std::string false_label = "condfalse." + std::to_string(instruction_number);
    std::string end_label = "condend."+std::to_string(instruction_number);
    c.change_block(true_label, output, 
        std::make_unique<basicblock::Cond_BR>(condition, true_label,false_label));
    auto t_value = true_expr->codegen(output, c);
    make_store(t_value,new_tmp, output, c);

    c.change_block(false_label, output,std::make_unique<basicblock::UCond_BR>(end_label));  
    auto f_value = false_expr->codegen(output, c);
    make_store(f_value,new_tmp, output, c);

    c.change_block(end_label,output,std::make_unique<basicblock::UCond_BR>(end_label));
    return make_load(new_tmp, output, c);
}

value::Value* IfStmt::codegen(std::ostream& output, context::Context& c)const {
    auto condition = if_condition->codegen(output, c);
    condition = codegen_convert(type::make_basic(type::IType::Bool),condition, output, c);
    int instruction_number = c.new_local_name(); 
    std::string true_label = "iftrue."+std::to_string(instruction_number);
    std::string end_label = "ifend."+std::to_string(instruction_number);
    std::string false_label;
    if(this->else_body.has_value()){
        false_label = "iffalse."+std::to_string(instruction_number);
    }else{
        false_label = "ifend."+std::to_string(instruction_number);
    }
    c.change_block(true_label, output, 
        std::make_unique<basicblock::Cond_BR>(condition, true_label,false_label));

    if_body->codegen(output, c);

    if(this->else_body.has_value()){
        c.change_block(false_label, output,std::make_unique<basicblock::UCond_BR>(end_label));
        else_body.value()->codegen(output, c);
    }
    c.change_block(end_label,output,std::make_unique<basicblock::UCond_BR>(end_label));
    return nullptr;
}
value::Value* CompoundStmt::codegen(std::ostream& output, context::Context& c)const {
    c.enter_scope();
    for(const auto& stmt : stmt_body){
        stmt->codegen(output, c);
    }
    c.exit_scope();
    return nullptr;
}
value::Value* FunctionDef::codegen(std::ostream& output, context::Context& c)const {
    assert(return_type == type::make_basic(type::IType::Int));
    AST::print_whitespace(c.depth(), output);
    output << "define "<<type::ir_type(return_type)<<" @" + name+"(){"<<std::endl;
    c.enter_function(return_type, output);
    function_body->codegen(output, c);
    c.exit_function(output);
    AST::print_whitespace(c.depth(), output);
    output << "}"<<std::endl;
    //Ultimately return value with
    //full function signature type
    //Once we add function argument/function types
    return nullptr;
}

value::Value* ReturnStmt::codegen(std::ostream& output, context::Context& c)const {
    auto return_value = return_expr->codegen(output, c);
    return_value = codegen_convert(c.return_type(),std::move(return_value), output, c);
    int instruction_number = c.new_local_name(); 
    c.change_block("afterret."+std::to_string(instruction_number),output, 
        std::make_unique<basicblock::RET>(return_value));
    return nullptr;
}

value::Value* Variable::codegen(std::ostream& output, context::Context& c)const {
    assert(this->analyzed && "This AST node has not had analysis run on it");
    auto var_value = c.get_value(variable_name);
    return make_load(var_value,output,c);
}

value::Value* DeclList::codegen(std::ostream& output, context::Context& c)const {
    for(const auto& decl : decls){
        decl->codegen(output, c);
    }
    return nullptr;
}
value::Value* VarDecl::codegen(std::ostream& output, context::Context& c)const {
    assert(this->analyzed && "This AST node has not had analysis run on it");
    auto variable = c.add_local(name, type);
    AST::print_whitespace(c.depth(), output);
    output << variable->get_value() <<" = alloca "<<type::ir_type(variable->get_type()) <<std::endl;
    if(this->assignment.has_value()){
        this->assignment.value()->codegen(output, c);
    }
    return nullptr;
}

value::Value* Constant::codegen(std::ostream& output, context::Context& c)const {
    assert(this->analyzed && "This AST node has not had analysis run on it");
    return c.add_literal(this->literal, this->type);
}

value::Value* Postfix::codegen(std::ostream& output, context::Context& c)const {
    assert(this->analyzed && "This AST node has not had analysis run on it");
    auto operand = arg->codegen(output, c);
    std::string t = type::ir_type(this->type);
    std::string command = "";
    value::Value* new_temp = nullptr;
    switch(tok.type){
        case token::TokenType::Plusplus:
        {
            auto variable = dynamic_cast<const ast::Variable*>(arg.get());
            assert(variable && "Other lvalues not yet implemented");
            auto var_reg = c.get_value(variable->variable_name);
            new_temp = make_load(var_reg, output, c);
            command = std::visit(overloaded{
                [](type::IType){return "add";},
                [](type::FType){return "fadd";},
                }, this->type);
            
            AST::print_whitespace(c.depth(), output);
            auto var_temp = c.new_temp(this->type);
            output << var_temp->get_value()<<" = "<<command<<" "<<t<<" "<<new_temp->get_value()<<std::visit(overloaded{
                [](type::IType){return ", 1";},
                [](type::FType){return ", 1.0";},
                }, this->type) <<std::endl;
            make_store(var_temp,var_reg, output, c);
        }
            return new_temp;
        case token::TokenType::Minusminus:
        {
            auto variable = dynamic_cast<const ast::Variable*>(arg.get());
            assert(variable && "Other lvalues not yet implemented");
            auto var_reg = c.get_value(variable->variable_name);
            new_temp = make_load(var_reg, output, c);
            command = std::visit(overloaded{
                [](type::IType){return "sub";},
                [](type::FType){return "fsub";},
                }, this->type);
            
            AST::print_whitespace(c.depth(), output);
            auto var_temp = c.new_temp(this->type);
            output << var_temp->get_value()<<" = "<<command<<" "<<t<<" "<<new_temp->get_value()<<std::visit(overloaded{
                [](type::IType){return ", 1";},
                [](type::FType){return ", 1.0";},
                }, this->type) <<std::endl;
            make_store(var_temp,var_reg, output, c);
        }
            return new_temp;
        default:
            assert(false && "Operator Not Implemented");
    }
}
value::Value* UnaryOp::codegen(std::ostream& output, context::Context& c)const {
    assert(this->analyzed && "This AST node has not had analysis run on it");
    auto operand = arg->codegen(output, c);
    std::string t = type::ir_type(this->type);
    std::string command = "";
    value::Value* new_temp = nullptr;
    switch(tok.type){
        case token::TokenType::Plusplus:
        {
            auto variable = dynamic_cast<const ast::Variable*>(arg.get());
            assert(variable && "Other lvalues not yet implemented");
            auto var_reg = c.get_value(variable->variable_name);
            auto var_temp = make_load(var_reg, output, c);
            var_temp =  codegen_convert(this->type, var_temp, output, c);
            command = std::visit(overloaded{
                [](type::IType){return "add";},
                [](type::FType){return "fadd";},
                }, var_temp->get_type());
            
            AST::print_whitespace(c.depth(), output);
            new_temp = c.new_temp(this->type);
            output << new_temp->get_value()<<" = "<<command<<" "<<t<<" "<<var_temp->get_value()<<std::visit(overloaded{
                [](type::IType){return ", 1";},
                [](type::FType){return ", 1.0";},
                }, var_temp->get_type()) <<std::endl;
            make_store(new_temp,var_reg, output, c);
        }
            return new_temp;
        case token::TokenType::Minusminus:
        {
            auto variable = dynamic_cast<const ast::Variable*>(arg.get());
            assert(variable && "Other lvalues not yet implemented");
            auto var_reg = c.get_value(variable->variable_name);
            auto var_temp = make_load(var_reg, output, c);
            var_temp =  codegen_convert(this->type, var_temp, output, c);
            command = std::visit(overloaded{
                [](type::IType){return "sub";},
                [](type::FType){return "fsub";},
                }, var_temp->get_type());
            
            AST::print_whitespace(c.depth(), output);
            new_temp = c.new_temp(this->type);
            output << new_temp->get_value()<<" = "<<command<<" "<<t<<" "<<var_temp->get_value()<<std::visit(overloaded{
                [](type::IType){return ", 1";},
                [](type::FType){return ", 1.0";},
                }, var_temp->get_type()) <<std::endl;
            make_store(new_temp,var_reg, output, c);
        }
            return new_temp;
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
            output << new_temp->get_value()<<" = "<<command<<" "<<t<<std::visit(overloaded{
                [](type::IType){return " 0, ";},
                [](type::FType){return " 0.0, ";},
                }, operand->get_type()) <<operand->get_value() <<std::endl;
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

            new_temp = codegen_convert(this->type, intermediate_bool, output, c);
        }
            return new_temp;
        default:
            assert(false && "Operator Not Implemented");
    }
}


value::Value* BinaryOp::codegen(std::ostream& output, context::Context& c)const {
    assert(this->analyzed && "This AST node has not had analysis run on it");
    switch(tok.type){
        case token::TokenType::PlusAssign:
        case token::TokenType::MinusAssign:
        case token::TokenType::DivAssign:
        case token::TokenType::MultAssign:
        case token::TokenType::ModAssign:
        case token::TokenType::BAAssign:
        case token::TokenType::BOAssign:
        case token::TokenType::BXAssign:
        case token::TokenType::LSAssign:
        case token::TokenType::RSAssign:
        case token::TokenType::Assign:
            return assignment_codegen(this, output, c);
        case token::TokenType::And:
        case token::TokenType::Or:
            return short_circuit_codegen(this, output, c);
        case token::TokenType::Minus:
        case token::TokenType::Plus:
        case token::TokenType::Div:
        case token::TokenType::Mult:
        case token::TokenType::Mod:
        case token::TokenType::Equal:
        case token::TokenType::NEqual:
        case token::TokenType::Less:
        case token::TokenType::Greater:
        case token::TokenType::LEq:
        case token::TokenType::GEq:
        case token::TokenType::LShift:
        case token::TokenType::RShift:
        case token::TokenType::BitwiseAnd:
        case token::TokenType::BitwiseOr:
        case token::TokenType::BitwiseXor:
        case token::TokenType::Comma:
            return other_bin_op_codegen(this, output, c);
        default:
            assert(false && "Unknown binary assignment op during codegen");
    }
}

} //namespace ast
