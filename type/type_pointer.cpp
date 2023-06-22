#include "type/type_pointer.h"
namespace type{
DerivedType::DerivedType(PointerType p) 
    : type(std::make_unique<PointerType>(p)){
       }
bool is_compatible(const PointerType& type1, const PointerType& type2){
    return is_compatible(type1.underlying_type, type2.underlying_type);
}
std::string to_string(const PointerType& type){
    return "pointer to "+to_string(type.underlying_type);
}
bool can_assign(const PointerType& right, const PointerType& left){
    return std::visit(overloaded{
            [](const FuncType& type)-> bool{
                return false;
            },
            [left= left.pointed_type()](const auto& right)-> bool{
                return is_compatible(right, left)
                    || is_type<VoidType>(right)
                    || is_type<VoidType>(left);
            }
        }, right.pointed_type());
}
std::string ir_type(const PointerType& type){
    return "ptr";
    //return ir_type(type.underlying_type) + "*";
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
