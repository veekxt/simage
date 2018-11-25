
#include "crypt.h"
#include <cstddef>
#include <iostream>

using std::byte;
using std::cout;
using std::cin;
using std::endl;

int aesgcm256_crypt(unsigned char src[], int src_len, unsigned char des[], int &des_len, unsigned char key[],
                unsigned char nonce[]) {
    unsigned long long des_len_t = 0;
    if (crypto_aead_aes256gcm_encrypt(des, &des_len_t,
                                      src, src_len,
                                      nullptr, 0,
                                      NULL, nonce, key) != 0) {
        return -1;
    }
    des_len = des_len_t;
    return 0;
}

int aesgcm256_decrypt(unsigned char src[], int src_len, unsigned char des[], int &des_len, unsigned char key[],
                      unsigned char nonce[]) {
    unsigned long long des_len_t = 0;
    if (src_len < crypto_aead_aes256gcm_ABYTES ||
        crypto_aead_aes256gcm_decrypt(des, &des_len_t,
                                      NULL,
                                      src, src_len,
                                      nullptr,
                                      0,
                                      nonce, key) != 0) {
        return -1;
    }
    des_len = des_len_t;
    return 0;
}