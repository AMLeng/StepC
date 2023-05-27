#include "type.h"
namespace type{
template <class... Ts>
struct overloaded : Ts...{
    using Ts::operator()...;
};

template<class...Ts> overloaded(Ts ...) -> overloaded<Ts...>;

namespace{


} //namespace
CType make_type(VoidType v){
    return v;
}
CType make_type(BasicType b){
    return b;
}
CType make_type(DerivedType d){
    return d;
}
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

bool is_compatible(const CType& type1, const CType& type2){
    return std::visit(overloaded{
        [&type2](VoidType){return std::holds_alternative<VoidType>(type2);},
        [&type2](BasicType type1){return std::holds_alternative<BasicType>(type2)
                            && type1 == std::get<BasicType>(type2);},
        [&type2](const DerivedType& type1){return std::holds_alternative<DerivedType>(type2) 
                            && is_compatible(type1, std::get<DerivedType>(type2));},
    }, type1);
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
std::string to_string(const CType& type){
    return std::visit(overloaded{
        [](VoidType v)->std::string{return "void";},
        [](BasicType t)->std::string{return to_string(t);},
        [](const DerivedType& t){return to_string(t);},
    }, type);
}
} //namespace type
