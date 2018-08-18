#ifndef SIMAGE_CRYPT_H
#define SIMAGE_CRYPT_H
#include <sodium.h>
int aesgcm256_crypt(unsigned char src[], int src_len, unsigned char des[], int &des_len, unsigned char key[],
                    unsigned char nonce[]);
int aesgcm256_decrypt(unsigned char src[], int src_len, unsigned char des[], int &des_len, unsigned char key[],
                      unsigned char nonce[]);

#define my_aesgcm256_crypt(a,b,c,d,e,f) \
aesgcm256_crypt((unsigned char *)(a),b,(unsigned char *)(c),d,(unsigned char *)(e),(unsigned char *)(f))

#define my_aesgcm256_decrypt(a,b,c,d,e,f) \
aesgcm256_decrypt((unsigned char *)(a),b,(unsigned char *)(c),d,(unsigned char *)(e),(unsigned char *)(f))

#endif //SIMAGE_CRYPT_H
