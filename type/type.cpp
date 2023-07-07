#include "type/type_basic.h"
#include "type/type_derived.h"
#include "type/type_func.h"
#include "type/type_pointer.h"
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
bool can_cast(const CType& type1, const CType& type2){
    if(can_assign(type1,type2)){
        return true;
    }
    if(!is_type<PointerType>(type1)){
        //Must be an int for int to pointer conversion
        return is_type<IType>(type1) && is_type<PointerType>(type2);
    }
    if(!is_type<PointerType>(type2)){
        //Must be an int for pointer to int conversion
        return is_type<IType>(type2) && is_type<PointerType>(type1);
    }
    return is_type<FuncType>(type::get<PointerType>(type1).pointed_type()) 
            == is_type<FuncType>(type::get<PointerType>(type2).pointed_type());
}
bool can_assign(const CType& type1, const CType& type2){
    //Leaves out the case of assigning a nullptr constant,
    //Since that requires the actual value and not just the type
    return std::visit(make_visitor<bool>(
        [&type2](VoidType){return std::holds_alternative<VoidType>(type2);},
        [&type2](BasicType type1){
            return is_type<BasicType>(type2);
            },
        [&type2](PointerType type1){return type2 == CType(IType::Bool)
            || is_type<PointerType>(type2) && can_assign(type1, type::get<PointerType>(type2));},
        [&type2](FuncType type1){return is_type<FuncType>(type2);}
    ),type1);
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
    return type::is_type<type::FType>(type);
}
bool is_int(CType type){
    return type::is_type<type::IType>(type);
}
bool is_arith(CType type){
    return type::is_type<type::BasicType>(type);
}
bool is_scalar(CType type){
    return type::is_type<type::BasicType>(type) || type::is_type<type::PointerType>(type);
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
        [](const FuncType& ft){return ft.ir_type();},
        [](const PointerType& pt){return pt.ir_type();},
        [](const ArrayType& at){return at.ir_type();}
    ), type);
}
int size(const CType& type){
    return std::visit(make_visitor<int>(
        [](VoidType v){return 0;},
        [](BasicType bt){return byte_size(bt);},
        [](const FuncType& ft){throw std::runtime_error("Cannot take size of function type");},
        [](const PointerType& pt){return 8;},
        [](const ArrayType& at){return at.size()*type::size(at.pointed_type());}
    ), type);
}

} //namespace type
