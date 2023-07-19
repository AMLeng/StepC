#include "symbol.h"
namespace symbol{
template <class... Ts>
struct overloaded : Ts...{
    using Ts::operator()...;
};

template<class...Ts> overloaded(Ts ...) -> overloaded<Ts...>;

std::set<std::optional<unsigned long long int>>* BlockTable::get_switch() const{
    if(switch_cases != nullptr) return switch_cases.get();
    BlockTable* p = dynamic_cast<BlockTable*>(parent);
    if(p == nullptr) return nullptr;
    return p->get_switch();
}
STable* STable::most_recent_child(){
    return children.back().get();
}
bool BlockTable::in_switch() const{
    return get_switch() != nullptr;
}
bool GlobalTable::in_function() const {
    return false;
}
bool BlockTable::in_function() const {
    return true;
}
void BlockTable::add_case(std::optional<unsigned long long int> case_val){
    auto current_switch = get_switch();
    assert(current_switch && "Can't add case outside of switch statement");
    auto insert_pair = current_switch->insert(case_val);
    if(insert_pair.second == false){
        throw std::runtime_error("Case already defined for this switch statement");
    }
}
BlockTable* BlockTable::new_block_scope_child(){
    children.push_back(std::make_unique<BlockTable>(this->global, this->current_func, this));
    return dynamic_cast<BlockTable*>(children.back().get());
}
BlockTable* BlockTable::new_switch_scope_child(){
    auto child = new_block_scope_child();
    child->switch_cases = std::make_unique<std::set<std::optional<unsigned long long int>>>();
    return child;
}
std::unique_ptr<std::set<std::optional<unsigned long long int>>> BlockTable::transfer_switch_table(){
    return std::move(switch_cases);
}
FuncTable* GlobalTable::new_function_scope_child(type::CType t){
    children.push_back(std::make_unique<FuncTable>(this, t));
    //Check that we're not already in a function
    return dynamic_cast<FuncTable*>(children.back().get());
}
std::optional<token::Token> BlockTable::unmatched_label() const{
    for(const auto& map_pair : current_func->function_labels){
        if(map_pair.second){
            return map_pair.second;
        }
    }
    return std::nullopt;
}
void BlockTable::require_label(const token::Token& tok){
    assert(current_func != nullptr && "Somehow have block not in function");
    if(current_func->function_labels.find(tok.value) == current_func->function_labels.end()){
        current_func->function_labels.emplace(tok.value,tok);
    }
}
void BlockTable::add_label(const std::string& name){
    assert(current_func != nullptr && "Somehow have block not in function");
    if(current_func->function_labels.find(name) != current_func->function_labels.end() 
            && current_func->function_labels.at(name) == std::nullopt){
        throw std::runtime_error("Label "+name+" already present in symbol table for this function");
    }
    current_func->function_labels.insert_or_assign(name,std::nullopt);
}
void STable::add_symbol(std::string name, type::CType type, bool has_def){
    //Symbol checking
    if(sym_map.find(name) != sym_map.end()){
        auto existing_decl = sym_map.at(name);
        if(this->parent != nullptr){
            throw std::runtime_error("Symbol "+name+" of incompatible type already present in symbol table at non-global scope");
        }
        if(has_def && existing_decl.second){
            throw std::runtime_error("Symbol "+name+" already defined at global scope");
        }
        if(!type::is_compatible(existing_decl.first, type)){
            throw std::runtime_error("Global symbol "+name+" of incompatible type already present in symbol table");
        }else{
            //type = type::make_composite(type, existing_type);
        }
    }
    sym_map.emplace(name,std::make_pair(type,has_def));
}
void GlobalTable::add_extern_decl(const std::string& name, const type::CType& type) {
    if(external_type_map.find(name) != external_type_map.end()){
        auto existing_type = external_type_map.at(name);
        if(!type::is_compatible(existing_type, type)){
            throw std::runtime_error("Global symbol "+name+" of incompatible type already present in symbol table");
        }else{
            //type = type::make_composite(type, existing_type);
        }
    }
    external_type_map.emplace(name,type);
}
void BlockTable::add_extern_decl(const std::string& name, const type::CType& type) {
    global->add_extern_decl(name, type);
}
bool BlockTable::tag_declared(std::string tag) const{
    if(this->tags.find(tag) != this->tags.end()){
        return true;
    }
    return parent->tag_declared(tag);
}
bool GlobalTable::tag_declared(std::string tag) const{
    return this->tags.find(tag) != this->tags.end();
}
type::CType BlockTable::get_tag(std::string tag) const{
    return global->get_tag(this->mangle_name(tag));
}
type::CType GlobalTable::get_tag(std::string tag) const{
    try{
        return this->tags.at(tag);
    }catch(std::runtime_error& e){
        throw std::runtime_error("Tag "+tag+" not found in symbol table");
    }
}
std::string BlockTable::mangle_name(std::string name) const{
    if(this->tags.find(name) != this->tags.end()){
        return  name + "."+std::to_string(this->tags.at(name));
    }
    return parent->mangle_name(name);
}
std::string GlobalTable::mangle_name(std::string name) const{
    if(!tag_declared(name)){
        throw std::runtime_error("Tag "+name+" not found in symbol table");
    }
    return name;
}
type::CType STable::mangle_type_or_throw(type::CType type) const{
    return std::visit(type::make_visitor<type::CType>(
        [](type::BasicType t){return t;},
        [](type::VoidType t){return t;},
        [&](const type::FuncType& t){
            auto ret = this->mangle_type_or_throw(t.return_type());
            if(t.has_prototype()){
                auto params = t.param_types();
                for(auto& param : params){
                    param = this->mangle_type_or_throw(param);
                }
                return type::FuncType(ret, params,t.is_variadic());
            }else{
                return type::FuncType(ret);
            }
        },
        [&](const type::PointerType& t){
            return type::PointerType(this->mangle_type_or_throw(t.pointed_type()));
        },
        [&](const type::ArrayType& t){
            if(t.is_complete()){
                return type::ArrayType(this->mangle_type_or_throw(t.pointed_type()), t.size());
            }else{
                return type::ArrayType(this->mangle_type_or_throw(t.pointed_type()), std::nullopt);
            }
        },
        [&](const type::StructType& t){
            auto mangled_tag = this->mangle_name(t.tag);
            if(t.is_complete()){
                auto mangled_members = t.members;
                for(auto& member : mangled_members){
                    member = this->mangle_type_or_throw(member);
                }
                return type::StructType(mangled_tag, mangled_members,t.indices);
            }else{
                return type::StructType(mangled_tag);
            }
        }
    ), type);
}
void GlobalTable::add_tag(std::string tag, type::TagType type){
    std::visit(type::overloaded{
        [&](type::StructType t)->void{
            if(this->tags.find(tag) != this->tags.end()){
                auto existing = this->tags.at(tag);
                if(!type::is_type<type::StructType>(existing)){
                    throw std::runtime_error("Tag "+tag+" already declared with different type");
                }
                if(t.is_complete() && type::get<type::StructType>(existing).is_complete()){
                    throw std::runtime_error("Tag "+tag+" already defined");
                }
            }else{
                this->tags.emplace(tag, t);
            }
        }
    }, type);
}
void BlockTable::add_tag(std::string tag, type::TagType type){
    std::visit(type::overloaded{
        [&](type::StructType t){
            if(tags.find(tag) == tags.end()){
                this->global->local_tag_count[tag] += 1; 
                this->tags.emplace(tag, this->global->local_tag_count[tag]);
            }
            auto mangled_tag = tag +"."+std::to_string(this->tags.at(tag));
            this->global->add_tag(mangled_tag, type::get<type::StructType>(this->mangle_type_or_throw(t)));
        }
    }, type);
}
std::map<std::string, type::CType> GlobalTable::get_tags(){
    return tags;
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

type::CType STable::symbol_type(std::string name) const{
    const STable* to_search = this;
    while(to_search != nullptr){
        if(to_search->sym_map.find(name) != to_search->sym_map.end()){
            return to_search->sym_map.at(name).first;
        }
        to_search = to_search->parent;
    }
    throw std::runtime_error("Symbol "+name+" not found in symbol table");
    __builtin_unreachable();
}
type::CType BlockTable::return_type(){
    assert(current_func && "Somehow have block outside of function");
    return current_func ->ret_type;
}
} //namespace symbol
