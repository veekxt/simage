
#ifndef SIMAGE_UTILS_H
#define SIMAGE_UTILS_H

#include <cstddef>
#include <iostream>
#include <unistd.h>

using std::byte;
using std::cout;
using std::cin;
using std::endl;

uint64_t get_timestamp(void);

void pbytes(byte bs[], int len);

void pbytess(byte bs[], int len);

#define int64_byte(b, n)\
do{\
for(int i=0;i<8;i++){\
b[i] =  byte(n>>(i*8));\
}\
}while(0)

#define byte_int64(b, n)\
do{\
n=0;\
for(int i=0;i<8;i++){\
n |= (uint64_t)(b)[i]<<(8*i);\
}\
}while(0)

#define g_insert(s,pos,a,len)\
do{\
    for(int i=0;i<len ;i++){\
        s[pos+i] = byte(a[i]);\
    }\
    pos+=len;\
}while(0);

void data_copy(int c, int s);

#endif //SIMAGE_UTILS_H
