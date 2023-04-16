#ifndef _CONTEXT_
#define _CONTEXT_
#include <string>
#include <cassert>
namespace context{
class Context{
    int number_of_temp;
public:
    int current_depth;
    Context() : number_of_temp(0), current_depth(0) {}
    std::string prev_temp(int i) const{
        assert(i < number_of_temp);
        return "%" + std::to_string(number_of_temp - i);
    }
    std::string new_temp(){
        number_of_temp++;
        return prev_temp(0);
    }
};
} //namespace context
#endif
