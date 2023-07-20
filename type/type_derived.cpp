#include "type/type_derived.h"
#include "type/type_func.h"
#include "type.h"
namespace type{

namespace{

} //namespace
DerivedType::DerivedType(const DerivedType& other){
    this->type = std::visit(overloaded{
        [](const auto& p)-> DerivedPointers {return p->copy();}
    }, other.type);
}
DerivedType& DerivedType::operator=(const DerivedType& other){
    this->type = std::visit(overloaded{
        [](const auto& p)-> DerivedPointers {return p->copy();}
    }, other.type);
    return *this;
}

bool DerivedType::operator ==(const DerivedType& other) const{
    return std::visit(overloaded{
        //Cases for non-array types are all the same
        [&type2 = std::as_const(other.type)](const std::unique_ptr<UnionType>& type1){
            if(!std::holds_alternative<std::unique_ptr<UnionType>>(type2)){
                return false;
            }
            const auto& t2 = std::get<std::unique_ptr<UnionType>>(type2);
            assert(type1 && t2 && "Invalid derived type containing nullptr");
            return *type1 == *t2;
        },
        [&type2 = std::as_const(other.type)](const std::unique_ptr<StructType>& type1){
            if(!std::holds_alternative<std::unique_ptr<StructType>>(type2)){
                return false;
            }
            const auto& t2 = std::get<std::unique_ptr<StructType>>(type2);
            assert(type1 && t2 && "Invalid derived type containing nullptr");
            return *type1 == *t2;
        },
        [&type2 = std::as_const(other.type)](const std::unique_ptr<FuncType>& type1){
            if(!std::holds_alternative<std::unique_ptr<FuncType>>(type2)){
                return false;
            }
            const auto& t2 = std::get<std::unique_ptr<FuncType>>(type2);
            assert(type1 && t2 && "Invalid derived type containing nullptr");
            return *type1 == *std::get<std::unique_ptr<FuncType>>(type2);
            },
        [&type2 = std::as_const(other.type)](const std::unique_ptr<PointerType>& type1){
            if(!std::holds_alternative<std::unique_ptr<PointerType>>(type2)){
                return false;
            }
            const auto& t2 = std::get<std::unique_ptr<PointerType>>(type2);
            assert(type1 && t2 && "Invalid derived type containing nullptr");
            if(auto p = dynamic_cast<ArrayType*>(type1.get())){
                return *p == *dynamic_cast<ArrayType*>(std::get<std::unique_ptr<PointerType>>(type2).get());
            }else{
                return *type1 == *std::get<std::unique_ptr<PointerType>>(type2);
            }
            }
    }, this->type);
}
bool DerivedType::operator !=(const DerivedType& other) const{
    return !this->operator==(other);
}

bool is_compatible(const DerivedType& type1, const DerivedType& type2){
    return std::visit(overloaded{
        [&type2 = std::as_const(type2.type)](const std::unique_ptr<UnionType>& type1){
            if(!std::holds_alternative<std::unique_ptr<UnionType>>(type2)){
                return false;
            }
            const auto& t2 = std::get<std::unique_ptr<UnionType>>(type2);
            assert(type1 && t2 && "Invalid derived type containing nullptr");
            return *type1 == *t2;
        },
        [&type2 = std::as_const(type2.type)](const std::unique_ptr<StructType>& type1){
            if(!std::holds_alternative<std::unique_ptr<StructType>>(type2)){
                return false;
            }
            const auto& t2 = std::get<std::unique_ptr<StructType>>(type2);
            assert(type1 && t2 && "Invalid derived type containing nullptr");
            return *type1 == *t2;
        },
        [&type2 = std::as_const(type2.type)](const std::unique_ptr<FuncType>& type1){
            if(!std::holds_alternative<std::unique_ptr<FuncType>>(type2)){
                return false;
            }
            const auto& t2 = std::get<std::unique_ptr<FuncType>>(type2);
            assert(type1 && t2 && "Invalid derived type containing nullptr");
            return is_compatible(*type1, *std::get<std::unique_ptr<FuncType>>(type2));
            },
        [&type2 = std::as_const(type2.type)](const std::unique_ptr<PointerType>& type1){
            if(!std::holds_alternative<std::unique_ptr<PointerType>>(type2)){
                return false;
            }
            const auto& t2 = std::get<std::unique_ptr<PointerType>>(type2);
            assert(type1 && t2 && "Invalid derived type containing nullptr");
            if(auto p1 = dynamic_cast<ArrayType*>(type1.get())){
                auto p2 = dynamic_cast<ArrayType*>(std::get<std::unique_ptr<PointerType>>(type2).get());
                if(!p2){
                    return false;
                }
                return is_compatible(*p1, *p2);
            }else{
                return is_compatible(*type1, *std::get<std::unique_ptr<PointerType>>(type2));
            }
        }
    }, type1.type);
}

std::string to_string(const DerivedType& arg){
    try{
        return std::visit(overloaded{
            [](const auto& type)->std::string{
                return type->to_string();
            }
        }, arg.type);
    }catch(std::exception& e){
        throw std::runtime_error("Failed to convert DerivedType to string");
    }
}
}//namespace type
