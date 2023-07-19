#include "type.h"
namespace type{
DerivedType::DerivedType(StructType p) 
    : type(std::make_unique<StructType>(p)){
       }
std::string StructType::to_string() const{
    if(members.size() > 0){
        std::string s = "Struct "+tag+ " {";
        for(int i=0; i<members.size()-1; i++){
            s += type::to_string(members.at(i)) +", ";
        }
        s += type::to_string(members.back()) + "}";
        return s;
    }else{
        return "Opaque Struct "+tag;
    }
}
std::string StructType::ir_type() const{
    if(members.size() > 0){
        std::string s = "<{";
        for(int i=0; i<members.size()-1; i++){
            s += type::ir_type(members.at(i)) +", ";
        }
        s += type::ir_type(members.back()) + "}>";
        return s;
    }else{
        return "%"+tag;
    }
}
long long int StructType::size() const{
    int size = 0;
    for(const auto& member : members){
        size += type::size(member);
    }
    return size;
}
bool StructType::operator ==(const StructType& other) const{
    return tag == other.tag 
        && members == other.members
        && indices == other.indices;
}
bool StructType::operator !=(const StructType& other) const{
    return !this->operator==(other);
}
std::unique_ptr<StructType> StructType::copy() const{
    return std::make_unique<StructType>(*this);
}
bool StructType::is_complete() const{
    return members.size() > 0;
}
}
