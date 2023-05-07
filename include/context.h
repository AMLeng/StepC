#ifndef _CONTEXT_
#define _CONTEXT_
#include <string>
#include <cassert>
#include <memory>
#include "type.h"
namespace context{
class Context{
    int number_of_temp;
    int current_depth;
    std::unique_ptr<type::BasicType> ret_type;
public:
    Context() : number_of_temp(0), current_depth(0) {}
    std::string prev_temp(int i) const{
        assert(i < number_of_temp);
        return "%" + std::to_string(number_of_temp - i);
    }
    std::string new_temp(){
        number_of_temp++;
        return prev_temp(0);
    }
    void enter_function(type::BasicType t){
        current_depth++;
        ret_type = std::make_unique<type::BasicType>(t);
    }
    void exit_function(){
        current_depth--;
        ret_type = nullptr;
    }
    int depth() const{
        return current_depth;
    }
    type::BasicType return_type(){
        assert(ret_type);
        return *ret_type;
    }
};
} //namespace context
#endif
