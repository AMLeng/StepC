#ifndef _CONTEXT_
#define _CONTEXT_
#include <string>
namespace context{
class Context{
    int number_of_temp;
public:
    Context() : number_of_temp(0) {}
    std::string last_temp() const{
        return std::to_string(number_of_temp - 1);
    }
    std::string new_temp(){
        number_of_temp++;
        return last_temp();
    }
};
} //namespace context
#endif
