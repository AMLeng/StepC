#include "type.h"
namespace type{
UnionType::UnionType(std::string tag, std::vector<CType> members, std::map<std::string, int> indices) :
    tag(tag), members(members), indices(indices), complete(true), largest_computed(false){
}
DerivedType::DerivedType(UnionType p) 
    : type(std::make_unique<UnionType>(p)){
       }
std::string UnionType::to_string() const{
    if(members.size() > 0){
        std::string s = "Union "+tag+ " {";
        for(int i=0; i<members.size()-1; i++){
            s += type::to_string(members.at(i)) +", ";
        }
        s += type::to_string(members.back()) + "}";
        return s;
    }else{
        if(complete){
            return "Union "+tag+" {}";
        }
        return "Opaque Union "+tag;
    }
}
std::string UnionType::ir_type() const{
    if(members.size() > 0){
        assert(largest_computed && "Cannot compute ir type without computing largest member");
        return "{" +type::ir_type(largest)+"}";
    }else{
        if(complete){
            return "{}";
        }
        return "%"+tag;
    }
}
void UnionType::compute_largest(const std::map<std::string, type::CType>& tags){
    if(!is_complete()){
        throw std::runtime_error("Cannot compute largest element of incomplete union "+this->tag);
    }else{
        largest_computed = true;
        int size = 0;
        for(const auto& member : members){
            auto member_size = type::size(member);
            if(member_size > size){
                size = member_size;
                largest = member;
            }
        }
    }
}
long long int UnionType::size(const std::map<std::string, type::CType>& tags) const{
    if(!is_complete()){
        try{
            auto type = tags.at(this->tag);
            return type::size(type);
        }catch(std::exception& e){
            throw std::runtime_error("Cannot take size of incomplete or undefined union "+this->tag);
        }
    }else{
        int size = 0;
        int align = 0;
        for(const auto& member : members){
            auto member_size = type::size(member);
            if(member_size > size){
                size = member_size;
            }
            auto member_align = type::align(member);
            if(member_align > align){
                align = member_align;
            }
        }
        if(size % align != 0){
            size = ((size/align) + 1)*align;
        }
        return size;
    }
}
long long int UnionType::align(const std::map<std::string, type::CType>& tags) const{
    if(!is_complete()){
        try{
            auto type = tags.at(this->tag);
            return type::align(type);
        }catch(std::exception& e){
            throw std::runtime_error("Cannot take alignment of incomplete or undefined union "+this->tag);
        }
    }else{
        int align = 0;
        for(const auto& member : members){
            auto member_align = type::align(member);
            if(member_align > align){
                align = member_align;
            }
        }
        return align;
    }
}
bool UnionType::operator ==(const UnionType& other) const{
    if(complete && other.complete){
        return tag == other.tag 
            && members == other.members
            && indices == other.indices;
    }else{
        return tag == other.tag ;
    }
}
bool UnionType::operator !=(const UnionType& other) const{
    return !this->operator==(other);
}
std::unique_ptr<UnionType> UnionType::copy() const{
    return std::make_unique<UnionType>(*this);
}
bool UnionType::is_complete() const{
    return complete;
}
}
