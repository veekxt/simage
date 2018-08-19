
#ifndef SIMAGE_UTILS_H
#define SIMAGE_UTILS_H

#include <cstddef>
#include <iostream>
#include <unistd.h>

using std::byte;
using std::cout;
using std::cin;
using std::endl;

extern byte g_key[];

uint64_t get_timestamp(void);

void pbytes(byte bs[], int len);

void pbytess(byte bs[], int len);

#define int2byte(b, n,j)\
do{\
for(int i=0;i<j;i++){\
(b)[i] =  byte(n>>(i*8));\
}\
}while(0)

#define byte2int(b, n,j)\
do{\
n=0;\
for(int i=0;i<j;i++){\
n |= (uint64_t)(b)[i]<<(8*i);\
}\
}while(0)

#define g_insert(s,pos,a,len)\
do{\
    for(int i=0;i<len ;i++){\
        s[pos+i] = byte((a)[i]);\
    }\
    pos+=len;\
}while(0);

void data_copy(int c, int s);
template<typename T>
void del_pre(T s, int n);
void data_copy_safe(int c, int s, byte en_nonce[], byte de_nonce[]);
int getopt_from(int &port, char *key, int &is_client, char *, int argc, char *argv[]);

#endif //SIMAGE_UTILS_H
