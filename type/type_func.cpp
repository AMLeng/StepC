#include "type_func.h"
namespace type{
DerivedType::DerivedType(FuncType f) 
    : type(std::make_unique<FuncType>(f)){
       }

bool FuncType::has_prototype() const{
    return prototype.has_value();
}
CType make_type(FuncType f){
    return make_type(DerivedType(f));
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
std::string FuncType::to_string(const FuncType::FuncPrototype& t){
    std::string s = "(";
    if(t.param_types.size() > 0){
        for(int i=0; i<t.param_types.size()-1; i++){
            s += type::to_string(t.param_types.at(i));
            s+= ",";
        }
        s += type::to_string(t.param_types.back());
    }
    if(t.variadic){
        s+=",...";
    }
    s += ")";
    return s;
}
std::string to_string(const FuncType& t){
    if(t.prototype.has_value()){
        return to_string(t.ret_type) + FuncType::to_string(t.prototype.value());
    }else{
        return to_string(t.ret_type) + "()";
    }
}

}//namespace type
