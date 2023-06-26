#include "type.h"
namespace type{

DerivedType::DerivedType(ArrayType p) 
    : type(std::make_unique<ArrayType>(p)){
       }
bool is_compatible(const ArrayType& type1, const ArrayType& type2){
    if(is_compatible(type1.underlying_type, type2.underlying_type)){
        if(!type1.is_complete() || !type2.is_complete()){
            return true;
        }
        return type1.size() == type2.size();
    }
    return false;
}
std::string to_string(const ArrayType& type){
    return "array to "+to_string(type.underlying_type);
}
std::string ir_type(const ArrayType& type){
    return "ptr";
    //return ir_type(type.underlying_type) + "*";
}
bool ArrayType::operator ==(const ArrayType& other) const{
    return this->underlying_type == other.underlying_type;
}
bool ArrayType::operator !=(const ArrayType& other) const{
    return !this->operator==(other);
}
CType ArrayType::element_type() const{
    return this->underlying_type;
}
PointerType ArrayType::decay() const{
    return type::PointerType(this->underlying_type);
}
void ArrayType::set_size(int size){
    if(size <= 0){
        throw std::runtime_error("Cannot have array of non-positive size");
    }
    allocated_size = size;
}
int ArrayType::size() const{
    if(allocated_size <= 0){
        throw std::runtime_error("Size of array is undetermined");
    }
    return allocated_size;
}
bool ArrayType::is_complete() const{
    return allocated_size > 0;
}

}