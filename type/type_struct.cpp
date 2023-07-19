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
        if(complete){
            return "Struct "+tag+" {}";
        }
        return "Opaque Struct "+tag;
    }
}
std::string StructType::ir_type() const{
    if(members.size() > 0){
        std::string s = "{";
        for(int i=0; i<members.size()-1; i++){
            s += type::ir_type(members.at(i)) +", ";
        }
        s += type::ir_type(members.back()) + "}";
        return s;
    }else{
        if(complete){
            return "{}";
        }
        return "%"+tag;
    }
}
long long int StructType::size(const std::map<std::string, type::CType>& tags) const{
    if(!is_complete()){
        try{
            auto type = tags.at(this->tag);
            return type::size(type, tags);
        }catch(std::exception& e){
            throw std::runtime_error("Cannot take size of incomplete or undefined struct "+this->tag);
        }
    }else{
        int size = 0;
        int align = 0;
        for(const auto& member : members){
            auto member_size = type::size(member, tags);
            auto member_align = type::align(member, tags);
            if(size % member_align != 0){
                size = ((size/member_align) + 1) * member_align;
            }
            if(align < member_align){
                align = member_align;
            }
            size += member_size;
        }
        if(size % align != 0){
            size = ((size/align) + 1) * align;
        }
        return size;
    }
}
long long int StructType::align(const std::map<std::string, type::CType>& tags) const{
    if(!is_complete()){
        try{
            auto type = tags.at(this->tag);
            return type::align(type, tags);
        }catch(std::exception& e){
            throw std::runtime_error("Cannot take alignment of incomplete or undefined struct "+this->tag);
        }
    }else{
        int align = 0;
        for(const auto& member : members){
            auto member_align = type::align(member, tags);
            if(member_align > align){
                align = member_align;
            }
        }
        return align;
    }
}
bool StructType::operator ==(const StructType& other) const{
    if(complete && other.complete){
        return tag == other.tag 
            && members == other.members
            && indices == other.indices;
    }else{
        return tag == other.tag ;
    }
}
bool StructType::operator !=(const StructType& other) const{
    return !this->operator==(other);
}
std::unique_ptr<StructType> StructType::copy() const{
    return std::make_unique<StructType>(*this);
}
bool StructType::is_complete() const{
    return complete;
}
}
