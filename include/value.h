#ifndef _VALUE_
#define _VALUE_
#include <string>
namespace value{
class Value{
    std::string value;
public:
    Value(std::string value) : value(value) {}
    std::string get_value(){
        return value;
    }
};
} //namespace value
#endif
