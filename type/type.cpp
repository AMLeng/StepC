#include "type/type_basic.h"
#include "type/type_derived.h"
#include "type/type_func.h"
#include "type.h"
namespace type{

namespace{


} //namespace
bool is_compatible(const CType& type1, const CType& type2){
    return std::visit(overloaded{
        [&type2](VoidType){return std::holds_alternative<VoidType>(type2);},
        [&type2](BasicType type1){return std::holds_alternative<BasicType>(type2)
                            && type1 == std::get<BasicType>(type2);},
        [&type2](const DerivedType& type1){return std::holds_alternative<DerivedType>(type2) 
                            && is_compatible(type1, std::get<DerivedType>(type2));},
    }, type1);
}
bool can_convert(const CType& type1, const CType& type2){
    return std::visit(overloaded{
        [&type2](VoidType){return std::holds_alternative<VoidType>(type2);},
        [&type2](BasicType type1){return std::holds_alternative<BasicType>(type2);},
        [&type2](const DerivedType& type1){return std::holds_alternative<DerivedType>(type2) 
                            && can_convert(type1, std::get<DerivedType>(type2));},
    }, type1);
}
std::string to_string(const CType& type){
    return std::visit(overloaded{
        [](VoidType v)->std::string{return "void";},
        [](BasicType t)->std::string{return to_string(t);},
        [](const DerivedType& t){return to_string(t);},
    }, type);
}

bool is_signed_int(CType type){
    return std::holds_alternative<BasicType>(type) && is_signed_int(std::get<BasicType>(type));
}
bool is_unsigned_int(CType type){
    return std::holds_alternative<BasicType>(type) && is_unsigned_int(std::get<BasicType>(type));
}
bool is_float(CType type){
    return std::holds_alternative<BasicType>(type) && is_float(std::get<BasicType>(type));
}
bool is_int(CType type){
    return std::holds_alternative<BasicType>(type) && is_int(std::get<BasicType>(type));
}
bool is_arith(CType type){
    return std::holds_alternative<BasicType>(type) && is_arith(std::get<BasicType>(type));
}
bool is_scalar(CType type){
    return std::holds_alternative<BasicType>(type) && is_scalar(std::get<BasicType>(type));
}
BasicType usual_arithmetic_conversions(CType type1, CType type2){
    try{
        auto itype1 = std::get<BasicType>(type1);
        auto itype2 = std::get<BasicType>(type2);
        return usual_arithmetic_conversions(itype1, itype2);
    }catch(std::exception& e){
        throw std::runtime_error("Failure to do integer promotions on types "+type::to_string(type1) + "and "+type::to_string(type2));
    }
}
BasicType integer_promotions(const CType& type){
    try{
        auto itype = std::get<BasicType>(type);
        return integer_promotions(itype);
    }catch(std::exception& e){
        throw std::runtime_error("Failure to do integer promotions on type "+type::to_string(type));
    }
}
std::string ir_type(const CType& type){
    return std::visit(make_visitor<std::string>(
        [](VoidType v)->std::string{return "void";},
        [](BasicType bt)->std::string{return ir_type(bt);},
        [](const FuncType& ft){return ir_type(ft);}
    ), type);
}

} //namespace type
