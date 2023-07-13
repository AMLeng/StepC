#include "type.h"
namespace type{
std::string StructType::to_string() const{
    std::string s = "Struct {";
    for(int i=0; i<members.size()-1; i++){
        s += type::to_string(members.at(i)) +", ";
    }
    s += type::to_string(members.back()) + "}";
    return s;
}
std::string StructType::ir_type() const{
    std::string s = "<{";
    for(int i=0; i<members.size()-1; i++){
        s += type::ir_type(members.at(i)) +", ";
    }
    s += type::ir_type(members.back()) + "}>";
    return s;
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
}
