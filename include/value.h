#ifndef _VALUE_
#define _VALUE_
#include <string>
#include "type.h"
namespace value{
class Value{
    //The name of the corresponding SSA register, with global or local indicator
    const std::string value;
    const type::CType type;
public:
    Value(std::string value, type::CType type) : value(value), type(type) {}
    std::string get_value() const{
        return value;
    }
    /*std::string get_value_name() const{
        return value.substr(1);
    }*/
    type::CType get_type() const{
        return type;
    }
};
} //namespace value
#endif
