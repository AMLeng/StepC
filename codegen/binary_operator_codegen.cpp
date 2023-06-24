#include "codegen/codegen_utility.h"

namespace codegen_utility{
value::Value* bin_op_codegen(value::Value* left, value::Value* right, token::TokenType op_type, type::CType result_type, 
    std::ostream& output, context::Context& c){
    value::Value* result = nullptr;
    switch(op_type){
        case token::TokenType::Minus:
            result = std::visit(type::make_visitor<value::Value*>(
                [&](type::IType){return make_command(left->get_type(), "sub", left, right, output, c);},
                [&](type::FType){return make_command(left->get_type(), "fsub", left, right, output, c);},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [&](type::PointerType){
                    //IMPLEMENTATION DEFINED VALUE DEPENDENT ON HEADER stddef.h
                    left = convert(type::CType(type::IType::LLong), left, output, c);
                    right = convert(type::CType(type::IType::LLong), right, output, c);
                    return make_command(type::CType(type::IType::LLong), "sub", left, right, output, c);
                    },
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type());
            break;
        case token::TokenType::Plus:
            result = std::visit(type::make_visitor<value::Value*>(
                [&](type::IType){
                    if(type::is_type<type::IType>(right->get_type())){
                        return make_command(left->get_type(), "add", left, right, output, c);
                    }else{
                    //IMPLEMENTATION DEFINED VALUE DEPENDENT ON HEADER stddef.h
                        left = convert(type::CType(type::IType::LLong), left, output, c);
                        right = convert(type::CType(type::IType::LLong), right, output, c);
                        return make_command(type::CType(type::IType::LLong), "add", left, right, output, c);
                    }
                },
                [&](type::FType){return make_command(left->get_type(), "fadd", left, right, output, c);},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [&](type::PointerType){
                    //IMPLEMENTATION DEFINED VALUE DEPENDENT ON HEADER stddef.h
                    left = convert(type::CType(type::IType::LLong), left, output, c);
                    right = convert(type::CType(type::IType::LLong), right, output, c);
                    return make_command(type::CType(type::IType::LLong), "add", left, right, output, c);
                    },
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type());
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
            result = std::visit(type::make_visitor<value::Value*>(
                [&](type::IType){return make_command(type::from_str("_Bool"), "icmp eq", left, right, output, c);},
                [&](type::FType){return make_command(type::from_str("_Bool"), "fcmp oeq", left, right, output, c);},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [&](type::PointerType){
                    //IMPLEMENTATION DEFINED VALUE DEPENDENT ON HEADER stddef.h
                    left = convert(type::CType(type::IType::LLong), left, output, c);
                    right = convert(type::CType(type::IType::LLong), right, output, c);
                    auto diff = make_command(type::CType(type::IType::LLong), "sub", left, right, output, c);
                    print_whitespace(c.depth(), output);
                    auto intermediate_bool = c.new_temp(type::IType::Bool);
                    output << intermediate_bool->get_value() <<" = icmp eq "<<type::ir_type(diff->get_type());
                    output <<" 0, "<< diff->get_value()<<std::endl;
                    return intermediate_bool;
                    },
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type());
            break;
        case token::TokenType::NEqual:
            result = std::visit(type::make_visitor<value::Value*>(
                [&](type::IType){return make_command(type::from_str("_Bool"), "icmp ne", left, right, output, c);},
                [&](type::FType){return make_command(type::from_str("_Bool"), "fcmp one", left, right, output, c);},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [&](type::PointerType){
                    //IMPLEMENTATION DEFINED VALUE DEPENDENT ON HEADER stddef.h
                    left = convert(type::CType(type::IType::LLong), left, output, c);
                    right = convert(type::CType(type::IType::LLong), right, output, c);
                    auto diff = make_command(type::CType(type::IType::LLong), "sub", left, right, output, c);
                    print_whitespace(c.depth(), output);
                    auto intermediate_bool = c.new_temp(type::IType::Bool);
                    output << intermediate_bool->get_value() <<" = icmp ne "<<type::ir_type(diff->get_type());
                    output <<" 0, "<< diff->get_value()<<std::endl;
                    return intermediate_bool;
                    },
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type());
            break;
        case token::TokenType::Less:
            result = std::visit(type::make_visitor<value::Value*>(
                [&](type::IType t){
                    if(type::is_signed_int(t)){
                        return make_command(type::from_str("_Bool"), "icmp slt", left, right, output, c);
                    }else{
                        return make_command(type::from_str("_Bool"), "icmp ult", left, right, output, c);
                    }
                },
                [&](type::FType){return make_command(type::from_str("_Bool"), "fcmp olt", left, right, output, c);},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [&](type::PointerType){
                    //IMPLEMENTATION DEFINED VALUE DEPENDENT ON HEADER stddef.h
                    left = convert(type::CType(type::IType::LLong), left, output, c);
                    right = convert(type::CType(type::IType::LLong), right, output, c);
                    return make_command(type::from_str("_Bool"), "icmp slt", left, right, output, c);
                    },
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type());
            break;
        case token::TokenType::Greater:
            result = std::visit(type::make_visitor<value::Value*>(
                [&](type::IType t){
                    if(type::is_signed_int(t)){
                        return make_command(type::from_str("_Bool"), "icmp sgt", left, right, output, c);
                    }else{
                        return make_command(type::from_str("_Bool"), "icmp ugt", left, right, output, c);
                    }
                },
                [&](type::FType){return make_command(type::from_str("_Bool"), "fcmp ogt", left, right, output, c);},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [&](type::PointerType){
                    //IMPLEMENTATION DEFINED VALUE DEPENDENT ON HEADER stddef.h
                    left = convert(type::CType(type::IType::LLong), left, output, c);
                    right = convert(type::CType(type::IType::LLong), right, output, c);
                    return make_command(type::from_str("_Bool"), "icmp sgt", left, right, output, c);
                    },
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type());
            break;
        case token::TokenType::LEq:
            result = std::visit(type::make_visitor<value::Value*>(
                [&](type::IType t){
                    if(type::is_signed_int(t)){
                        return make_command(type::from_str("_Bool"), "icmp sle", left, right, output, c);
                    }else{
                        return make_command(type::from_str("_Bool"), "icmp ule", left, right, output, c);
                    }
                },
                [&](type::FType){return make_command(type::from_str("_Bool"), "fcmp ole", left, right, output, c);},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [&](type::PointerType){
                    //IMPLEMENTATION DEFINED VALUE DEPENDENT ON HEADER stddef.h
                    left = convert(type::CType(type::IType::LLong), left, output, c);
                    right = convert(type::CType(type::IType::LLong), right, output, c);
                    return make_command(type::from_str("_Bool"), "icmp sle", left, right, output, c);
                    },
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type());
            break;
        case token::TokenType::GEq:
            result = std::visit(type::make_visitor<value::Value*>(
                [&](type::IType t){
                    if(type::is_signed_int(t)){
                        return make_command(type::from_str("_Bool"), "icmp sge", left, right, output, c);
                    }else{
                        return make_command(type::from_str("_Bool"), "icmp uge", left, right, output, c);
                    }
                },
                [&](type::FType){return make_command(type::from_str("_Bool"), "fcmp oge", left, right, output, c);},
                [](type::FuncType){throw std::runtime_error("Cannot do operation on function type");},
                [&](type::PointerType){
                    //IMPLEMENTATION DEFINED VALUE DEPENDENT ON HEADER stddef.h
                    left = convert(type::CType(type::IType::LLong), left, output, c);
                    right = convert(type::CType(type::IType::LLong), right, output, c);
                    return make_command(type::from_str("_Bool"), "icmp sge", left, right, output, c);
                    },
                [](type::VoidType){throw std::runtime_error("Cannot do operation on void type");}
                ), left->get_type());
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
        case token::TokenType::Amp:
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
