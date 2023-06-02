#include "codegen/codegen_utility.h"

namespace codegen_utility{
value::Value* bin_op_codegen(value::Value* left, value::Value* right, token::TokenType op_type, type::CType result_type, 
    std::ostream& output, context::Context& c){
    value::Value* result = nullptr;
    switch(op_type){
        case token::TokenType::Minus:
            result = make_command(left->get_type(), std::visit(type::make_visitor<std::string>(
                [](type::IType){return "sub";},
                [](type::FType){return "fsub";},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [](type::PointerType){throw std::runtime_error("Pointer subtraction not yet implemented");},
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type()), left,right,output,c);
            break;
        case token::TokenType::Plus:
            result = make_command(left->get_type(),std::visit(type::make_visitor<std::string>(
                [](type::IType){return "add";},
                [](type::FType){return "fadd";},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [](type::PointerType){throw std::runtime_error("Pointer addition not yet implemented");},
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type()), left,right,output,c);
            break;
        case token::TokenType::Star:
            result = make_command(left->get_type(),std::visit(type::make_visitor<std::string>(
                [](type::IType){return "mul";},
                [](type::FType){return "fmul";},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [](type::PointerType){throw std::runtime_error("Cannot do operation on pointer type");},
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type()), left,right,output,c);
            break;
        case token::TokenType::Div:
            result = make_command(left->get_type(),std::visit(type::make_visitor<std::string>(
                [](type::IType t){
                    if(type::is_signed_int(t)){return "sdiv";}
                    else{return "udiv";}},
                [](type::FType){return "fdiv";},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [](type::PointerType){throw std::runtime_error("Cannot do operation on pointer type");},
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type()), left,right,output,c);
            break;
        case token::TokenType::Mod:
            result = make_command(left->get_type(),std::visit(type::make_visitor<std::string>(
                [](type::IType t){
                    if(type::is_signed_int(t)){return "srem";}
                    else{return "urem";}},
                [](type::FType){
                    assert(false && "C does not allow mod to take floating point arguments");
                    return "frem";},
                [](type::PointerType){throw std::runtime_error("Cannot do operation on pointer type");},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type()), left,right,output,c);
            break;
        case token::TokenType::Equal:
            result = make_command(type::from_str("_Bool"),std::visit(type::make_visitor<std::string>(
                [](type::IType){return "icmp eq";},
                [](type::FType){return "fcmp oeq";},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [](type::PointerType){throw std::runtime_error("Pointer equality not yet implemented (should be by difference)");},
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type()), left,right,output,c);
            break;
        case token::TokenType::NEqual:
            result = make_command(type::from_str("_Bool"),std::visit(type::make_visitor<std::string>(
                [](type::IType){return "icmp ne";},
                [](type::FType){return "fcmp one";},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [](type::PointerType){throw std::runtime_error("Pointer comparison not yet implemented");},
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type()), left,right,output,c);
            break;
        case token::TokenType::Less:
            result = make_command(type::from_str("_Bool"),std::visit(type::make_visitor<std::string>(
                [](type::IType t){
                    if(type::is_signed_int(t)){return "icmp slt";}
                    else{return "icmp ult";}},
                [](type::FType){return "fcmp olt";},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [](type::PointerType){throw std::runtime_error("Pointer comparison not yet implemented");},
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type()), left,right,output,c);
            break;
        case token::TokenType::Greater:
            result = make_command(type::from_str("_Bool"),std::visit(type::make_visitor<std::string>(
                [](type::IType t){
                    if(type::is_signed_int(t)){return "icmp sgt";}
                    else{return "icmp ugt";}},
                [](type::FType){return "fcmp ogt";},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [](type::PointerType){throw std::runtime_error("Pointer comparison not yet implemented");},
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type()), left,right,output,c);
            break;
        case token::TokenType::LEq:
            result = make_command(type::from_str("_Bool"),std::visit(type::make_visitor<std::string>(
                [](type::IType t){
                    if(type::is_signed_int(t)){return "icmp sle";}
                    else{return "icmp ule";}},
                [](type::FType){return "fcmp ole";},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [](type::PointerType){throw std::runtime_error("Pointer comparison not yet implemented");},
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type()), left,right,output,c);
            break;
        case token::TokenType::GEq:
            result = make_command(type::from_str("_Bool"),std::visit(type::make_visitor<std::string>(
                [](type::IType t){
                    if(type::is_signed_int(t)){return "icmp sge";}
                    else{return "icmp uge";}},
                [](type::FType){return "fcmp oge";},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [](type::PointerType){throw std::runtime_error("Pointer comparison not yet implemented");},
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type()), left,right,output,c);
            break;
        case token::TokenType::LShift:
            //LLVM IR requires both arguments to the shift to be the same integer type
            right = convert(left->get_type(), right, output, c);
            result = make_command(left->get_type(),"shl", left,right,output,c);
            break;
        case token::TokenType::RShift:
            //LLVM IR requires both arguments to the shift to be the same integer type
            right = convert(left->get_type(), right, output, c);
            //C standard says that if the left operand is signed and negative, then UB
            //So it doesn't matter if we lshr or ashr
            result = make_command(left->get_type(),"lshr", left,right,output,c);
            break;
        case token::TokenType::BitwiseAnd:
            result = make_command(left->get_type(),"and", left,right,output,c);
            break;
        case token::TokenType::BitwiseOr:
            result = make_command(left->get_type(),"or", left,right,output,c);
            break;
        case token::TokenType::BitwiseXor:
            result = make_command(left->get_type(),"xor", left,right,output,c);
            break;
        case token::TokenType::Comma:
            result = right;
            break;
        default:
            std::cerr << "Error on token of type "<<token::string_name(op_type) <<std::endl;
            assert(false && "Unknown binary op during codegen");
    }
    return convert(result_type, result, output, c);
}
} //namespace codegen_utility
