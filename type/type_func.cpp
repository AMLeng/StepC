#include "type_func.h"
namespace type{
DerivedType::DerivedType(FuncType f) 
    : type(std::make_unique<FuncType>(f)){
       }

DerivedType DerivedType::make_derived(FuncType f){
    return DerivedType(f);
}

bool FuncType::has_prototype() const{
    return prototype.has_value();
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

}//namespace type
