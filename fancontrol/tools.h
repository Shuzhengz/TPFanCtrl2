#include "_prec.h"

/// sets last byte to zero first 
inline size_t strlen_s(const char * str,const size_t SourceLen) {    
    (const_cast<char *>(str))[SourceLen-1] = 0;
    return strnlen(str,SourceLen);
}