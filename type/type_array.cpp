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
std::string ArrayType::to_string() const{
    if(allocated_size.has_value()){
        return "size "+std::to_string(allocated_size.value())+" array of "+type::to_string(this->underlying_type);
    }else{
        return "unknown size array of "+type::to_string(this->underlying_type);
    }
}
std::string ArrayType::ir_type() const{
    std::string size = "0";
    if(this->allocated_size.has_value()){
        size = std::to_string(this->allocated_size.value());
    }
    return "["+size+" x "+type::ir_type(this->underlying_type)+"]";
}
bool ArrayType::operator ==(const ArrayType& other) const{
    return this->underlying_type == other.underlying_type;
}
bool ArrayType::operator !=(const ArrayType& other) const{
    return !this->operator==(other);
}
std::unique_ptr<PointerType> ArrayType::copy() const{
    return std::make_unique<ArrayType>(*this);
}
CType ArrayType::element_type() const{
    return this->underlying_type;
}
void ArrayType::set_size(int size){
    if(size < 0){
        throw std::runtime_error("Cannot have array of negative size");
    }
    if(allocated_size.has_value()){
        throw std::runtime_error("Size of array is already set");
    }
    allocated_size = size;
}
int ArrayType::size() const{
    if(!allocated_size.has_value()){
        throw std::runtime_error("Size of array is undetermined");
    }
    return allocated_size.value();
}
bool ArrayType::is_complete() const{
    return allocated_size.has_value();
}

}
