#ifndef _BASIC_BLOCK_
#define _BASIC_BLOCK_
#include "value.h"
#include "type.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <memory>
namespace basicblock{
class Terminator{
public:
    virtual std::string get_instruction() = 0;
    virtual ~Terminator() = 0;
};

class RET : public Terminator{
    value::Value* ret_val;
public:
    RET(value::Value* v) : ret_val(v){}
    std::string get_instruction() override;
};
class Cond_BR : public Terminator{
    value::Value* cond;
    std::string t_label;
    std::string f_label;
public:
    Cond_BR(value::Value* cond, std::string tl, std::string fl);
    std::string get_instruction() override;
};

class UCond_BR : public Terminator{
    std::string label;
public:
    UCond_BR(std::string label) : label(label){}
    std::string get_instruction() override;
};
class Unreachable : public Terminator{
public:
    Unreachable(){}
    std::string get_instruction() override;
};

class Block{
    std::string label;
    std::unique_ptr<Terminator> t;
public:
    Block(std::string label) : label(label), t(nullptr) {}
    Block(std::string label, std::unique_ptr<Terminator> default_terminator) 
        : label(label), t(std::move(default_terminator)){}
    std::string get_label();
    bool has_terminator() const;
    void add_terminator(std::unique_ptr<Terminator> term);
    void print_terminator(std::ostream& output) const;
};

} //namespace basicblock
#endif
