#include "type.h"
#include <sstream>
#include <map>
namespace type{
ArrayType::ArrayType(CType t, std::optional<int> s) : PointerType(t), allocated_size(s){}

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
std::string ir_literal(const std::string& c_literal){
    static const std::map<char, std::string> special_chars = {{
        {'\a',"\\07"},{'\b',"\\08"},{'\f',"\\0C"},{'\n',"\\0A"},
        {'\r',"\\0D"},{'\t',"\\09"},{'\v',"\\0B"},{'\\',"\\5C"},
        {'\'',"\\27"},{'\"',"\\22"},{'\?',"\\3F"},{'\0',"\\00"}
    }};
    auto ss = std::stringstream{};
    ss << "c\"";
    for(const auto& c : c_literal){
        if(special_chars.find(c) != special_chars.end()){
            ss << special_chars.at(c);
        }else{
            ss << c;
        }
    }
    ss << "\"";
    return ss.str();
}
void ArrayType::set_size(long long int size){
    if(size < 0){
        throw std::runtime_error("Cannot have array of negative size");
    }
    if(allocated_size.has_value()){
        throw std::runtime_error("Size of array is already set");
    }
    allocated_size = size;
}
long long int ArrayType::size() const{
    if(!allocated_size.has_value()){
        throw std::runtime_error("Size of array is undetermined");
    }
    return allocated_size.value();
}
bool ArrayType::is_complete() const{
    return allocated_size.has_value();
}

}
