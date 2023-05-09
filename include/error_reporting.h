#ifndef _ERROR_REPORTING_
#define _ERROR_REPORTING_
#include <string>
#include <string_view>
namespace lexer{
    class Lexer;
} //namespace lexer
namespace error_reporting{
class Source{
    std::string str;
    int line;
    int col;
    friend class lexer::Lexer;
    void add(std::string_view s, int l, int c){
        str += s;
        line = l;
        col = c;
    }
    void print_to(std::ostream& os){
        os <<"source code: " <<std::endl;
        os << str <<std::endl;
        os <<"Ending at line "<<line<<" and column "<<col<<std::endl;
    }
public:
    Source() = default;
};

} //namespace error_reporting
#endif
