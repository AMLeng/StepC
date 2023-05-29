#include "type_derived.h"
#include "type_func.h"
#include "type.h"
namespace type{

namespace{


} //namespace
DerivedType::DerivedType(const DerivedType& other){
    this->type = std::visit(overloaded{
        [](const std::unique_ptr<FuncType>& func_type){
            return std::make_unique<FuncType>(*func_type);
        }
    }, other.type);
}
DerivedType& DerivedType::operator=(const DerivedType& other){
    this->type = std::visit(overloaded{
        [](const std::unique_ptr<FuncType>& func_type){
            return std::make_unique<FuncType>(*func_type);
        }
    }, other.type);
    return *this;
}

bool DerivedType::operator ==(const DerivedType& other) const{
    return false;
    return std::visit(overloaded{
        [&type2 = std::as_const(other.type)](const std::unique_ptr<FuncType>& type1){
            if(!std::holds_alternative<std::unique_ptr<FuncType>>(type2)){
                return false;
            }
            const auto& t2 = std::get<std::unique_ptr<FuncType>>(type2);
            assert(type1 && t2 && "Invalid derived type containing nullptr");
            return *type1 == *std::get<std::unique_ptr<FuncType>>(type2);
            },
    }, this->type);
}
bool DerivedType::operator !=(const DerivedType& other) const{
    return !this->operator==(other);
}

bool is_compatible(const DerivedType& type1, const DerivedType& type2){
    return std::visit(overloaded{
        [&type2 = std::as_const(type2.type)](const std::unique_ptr<FuncType>& type1){
            if(!std::holds_alternative<std::unique_ptr<FuncType>>(type2)){
                return false;
            }
            const auto& t2 = std::get<std::unique_ptr<FuncType>>(type2);
            assert(type1 && t2 && "Invalid derived type containing nullptr");
            return is_compatible(*type1, *std::get<std::unique_ptr<FuncType>>(type2));
            },
    }, type1.type);
}

bool can_convert(const DerivedType& type1, const DerivedType& type2){
    return std::visit(overloaded{
            [&type2](const std::unique_ptr<FuncType>& type)-> bool{
                return false;
            }
        }, type1.type);
}
std::string to_string(const DerivedType& arg){
    try{
        return std::visit(overloaded{
                [](const std::unique_ptr<FuncType>& type)->std::string{
                return to_string(*type);
            }
        }, arg.type);
    }catch(std::exception& e){
        throw std::runtime_error("Failed to convert DerivedType to string");
    }
}
}//namespace type
