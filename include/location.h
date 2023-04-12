#ifndef _Location_
#define _Location_
namespace location{
//Represents a location in the source code we're compiling, to use for debugging
struct Location{
    int start_line;
    int start_col;
    int end_line;
    int end_col;
};
} //namespace location
#endif
