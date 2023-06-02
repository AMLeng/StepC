#include "type/type_pointer.h"
namespace type{
DerivedType::DerivedType(PointerType p) 
    : type(std::make_unique<PointerType>(p)){
       }
bool is_compatible(const PointerType& type1, const PointerType& type2){
    return is_compatible(type1.underlying_type, type2.underlying_type);
}
bool can_convert(const PointerType& type1, const PointerType& type2){
    return std::visit(make_visitor<bool>(
        [&t2 = std::as_const(type2.underlying_type)](const FuncType& t1){
            return is_type<FuncType>(t2) /*&& is_compatible(t1, t2)*/;
            },
        [&t2 = std::as_const(type2.underlying_type)](const PointerType& t1){
            return (is_type<PointerType>(t2) && can_convert(t1, t2)) || is_type<VoidType>(t2);
            },
        [&t2 = std::as_const(type2.underlying_type)](const VoidType& ){
            return !is_type<FuncType>(t2);
            },
        [&t2 = std::as_const(type2.underlying_type)](const BasicType& ){
            return is_type<VoidType>(t2) || is_type<BasicType>(t2);
            }
    ), type1.underlying_type);
    return can_convert(type1.underlying_type, type2.underlying_type);
}
std::string to_string(const PointerType& type){
    return to_string(type.underlying_type) + "*";
}
std::string ir_type(const PointerType& type){
    return ir_type(type.underlying_type) + "*";
}
bool PointerType::operator ==(const PointerType& other) const{
    return this->underlying_type == other.underlying_type;
}
bool PointerType::operator !=(const PointerType& other) const{
    return !this->operator==(other);
}
CType PointerType::pointed_type() const{
    return this->underlying_type;
}
} //namespace type
