#include "symbol.h"
namespace symbol{
template <class... Ts>
struct overloaded : Ts...{
    using Ts::operator()...;
};

template<class...Ts> overloaded(Ts ...) -> overloaded<Ts...>;
std::set<std::optional<unsigned long long int>>* STable::get_switch() const{
    if(switch_cases != nullptr) return switch_cases.get();
    if(parent == nullptr) return nullptr;
    return parent->get_switch();
}
STable* STable::new_child(){
    auto child = STable(this);
    children.push_back(std::make_unique<STable>(std::move(child)));
    return children.back().get();
}
bool STable::in_switch() const{
    return get_switch() != nullptr;
}
void STable::add_case(std::optional<unsigned long long int> case_val){
    auto current_switch = get_switch();
    assert(current_switch && "Can't add case outside of switch statement");
    auto insert_pair = current_switch->insert(case_val);
    if(insert_pair.second == false){
        throw std::runtime_error("Case already defined for this switch statement");
    }
}
STable* STable::new_switch_scope_child(){
    auto child = new_child();
    child->switch_cases = std::make_unique<std::set<std::optional<unsigned long long int>>>();
    return child;
}
std::unique_ptr<std::set<std::optional<unsigned long long int>>> STable::transfer_switch_table(){
    return std::move(switch_cases);
}
STable* STable::new_function_scope_child(){
    auto child = new_child();
    //Check that we're not already in a function
    auto to_check = child;
    do{
        to_check = to_check->parent;
        if(to_check->function_labels != nullptr){
            throw std::runtime_error("Can't define function inside another function");
        }
    }while(to_check->parent != nullptr);
    child->function_labels = std::make_unique<std::map<std::string,std::optional<token::Token>>>();
    return child;
}
std::optional<token::Token> STable::unmatched_label() const{
    if(function_labels == nullptr){
        return parent->unmatched_label();
    }else{
        for(const auto& map_pair : *function_labels){
            if(map_pair.second){
                return map_pair.second;
            }
        }
        return std::nullopt;
    }
}
void STable::require_label(const token::Token& tok){
    if(function_labels == nullptr){
        if(parent == nullptr){
            throw std::runtime_error("Can't have label outside of function");
        }
        parent->require_label(tok);
    }else{
        if(function_labels->find(tok.value) == function_labels->end()){
            function_labels->emplace(tok.value,tok);
        }
    }
}
void STable::add_label(const std::string& name){
    if(function_labels == nullptr){
        if(parent == nullptr){
            throw std::runtime_error("Can't have label outside of function");
        }
        parent->add_label(name);
    }else{
        if(function_labels->find(name) != function_labels->end() && function_labels->at(name) == std::nullopt){
            throw std::runtime_error("Label "+name+" already present in symbol table for this function");
        }
        function_labels->insert_or_assign(name,std::nullopt);
    }
}
void STable::add_symbol(std::string name, type::CType type){
    //Scope checking
    if(std::holds_alternative<type::DerivedType>(type)){
        std::get<type::DerivedType>(type).visit(overloaded{
            [this](type::FuncType& ftype){
                if(this->parent != nullptr){
                    throw std::runtime_error("Cannot declare function unless at global scope");
                }
            },
        });
    }
    //Symbol checking
    if(sym_map.find(name) != sym_map.end()){
        auto existing_type = sym_map.at(name);
        if(this->parent != nullptr){
            throw std::runtime_error("Symbol "+name+" of incompatible type already present in symbol table at non-global scope");
        }
        if(!type::is_compatible(existing_type, type)){
            throw std::runtime_error("Global symbol "+name+" of incompatible type already present in symbol table");
        }else{
            //type = type::make_composite(type, existing_type);
        }
    }
    sym_map.emplace(name,type);
}
bool STable::has_symbol(std::string name){
    STable* to_search = this;
    while(to_search != nullptr){
        if(to_search->sym_map.find(name) != to_search->sym_map.end()){
            return true;
        }
        to_search = to_search->parent;
    }
    return false;
}
type::CType STable::symbol_type(std::string name){
    STable* to_search = this;
    while(to_search != nullptr){
        if(to_search->sym_map.find(name) != to_search->sym_map.end()){
            return to_search->sym_map.at(name);
        }
        to_search = to_search->parent;
    }
    throw std::runtime_error("Symbol "+name+" not found in symbol table");
    __builtin_unreachable();
}
} //namespace symbol
