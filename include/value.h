#ifndef _VALUE_
#define _VALUE_
#include <string>
#include "type.h"
namespace value{
class Value{
    //The name of the corresponding SSA register
    const std::string value;
    //ir type
    const type::BasicType type;
public:
    Value(std::string value, type::BasicType type) : value(value), type(type) {}
    std::string get_value() const{
        return value;
    }
    type::BasicType get_type() const{
        return type;
    }
};
} //namespace value
#endif
