#include "type/type_func.h"
namespace type{
DerivedType::DerivedType(FuncType f) 
    : type(std::make_unique<FuncType>(f)){
       }

FuncType::FuncType(CType ret, std::vector<CType> param, bool variadic)
    : ret_type(ret), prototype(std::make_optional<FuncPrototype>(param,variadic)) {
    auto void_type = type::CType{};
    for(const auto& param_type : param){
        if(is_compatible(param_type, void_type)){
            if(param.size() != 1 || variadic){
                throw std::runtime_error("Function with void arguments cannot have multiple arguments");
            }
        }
    }
    if(is_type<FuncType>(ret_type)){
        throw std::runtime_error("Function cannot return function type");
    }
}
FuncType::FuncType(CType ret)
    : ret_type(ret), prototype(std::nullopt) {
};
std::unique_ptr<FuncType> FuncType::copy() const{
    return std::make_unique<FuncType>(*this);
}
bool FuncType::has_prototype() const{
    return prototype.has_value();
}
bool FuncType::params_match(std::vector<CType> arg_types) const{
    if(!prototype.has_value()){
        return true;
    }
    auto params = prototype.value().param_types;
    if(params.size() > arg_types.size()){
        return false;
    } 
    if(params.size() < arg_types.size() && !prototype.value().variadic){
        return false;
    }
    for(int i=0; i<params.size(); i++){
        if(!can_assign(arg_types.at(i),params.at(i))){
            return false;
        }
    }
    return true;
}


bool is_compatible(const FuncType& type1, const FuncType& type2){
    if(!is_compatible(type1.ret_type, type2.ret_type)){
        return false;
    }
    if(type1.prototype.has_value() && type2.prototype.has_value()){
        if(type1.prototype->variadic != type2.prototype->variadic){
            return false;
        }
        int n = type1.prototype->param_types.size();
        if(n != type2.prototype->param_types.size()){
            return false;
        }
        for(int i=0; i<n; i++){
            if(!is_compatible(type1.prototype->param_types.at(i),
                type2.prototype->param_types.at(i))){
                return false;
            }
        }
    }
    return true;
}
bool FuncType::operator ==(const FuncType& other) const{
    if(!(ret_type == other.ret_type)){
        return false;
    }
    if(prototype.has_value() != other.prototype.has_value()){
        return false;
    }
    if(prototype.has_value()){
        if(prototype->variadic != other.prototype->variadic){
            return false;
        }
        int n = prototype->param_types.size();
        if(n != other.prototype->param_types.size()){
            return false;
        }
        for(int i=0; i<n; i++){
            if(!(prototype->param_types.at(i) == other.prototype->param_types.at(i))){
                return false;
            }
        }
    }
    return true;
}
bool FuncType::operator !=(const FuncType& other) const{
    return !this->operator==(other);
}

std::string FuncType::to_string(const FuncType::FuncPrototype& t){
    std::string s = "(";
    if(t.param_types.size() > 0){
        for(int i=0; i<t.param_types.size()-1; i++){
            s += type::to_string(t.param_types.at(i));
            s+= ",";
        }
        s += type::to_string(t.param_types.back());
        if(t.variadic){
            s+=",...";
        }
    }else{
        s+= "void";
    }
    s += ")";
    return s;
}
std::string FuncType::to_string() const{
    if(this->prototype.has_value()){
        return "function on " + FuncType::to_string(this->prototype.value()) + " returning "+type::to_string(this->ret_type);
    }else{
        return "function with unknown args returning "+type::to_string(this->ret_type);
    }
}
std::string FuncType::ir_type() const{
    std::string s = type::ir_type(this->ret_type)+"(";
    if(this->prototype.has_value()){
        auto t = this->prototype.value();
        if(t.param_types.size() > 0){
            for(int i=0; i<t.param_types.size()-1; i++){
                s += type::ir_type(t.param_types.at(i));
                s+= ",";
            }
            s += type::ir_type(t.param_types.back());
            if(t.variadic){
                s+=",...";
            }
        }
    }else{
        s+="...";
    }
    s+=")";
    return s;
}
bool FuncType::is_variadic() const{
    return has_prototype() && prototype.value().variadic;
}
std::vector<CType> FuncType::param_types() const{
    if(has_prototype()){
        return prototype.value().param_types;
    }else{
        throw std::runtime_error("Cannot get param types for function type without prototype");
    }
}
CType FuncType::return_type() const{
    return ret_type;
}

}//namespace type
