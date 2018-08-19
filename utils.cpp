
#include <sys/time.h>
#include <sys/socket.h>
#include "utils.h"
#include "crypt.h"
#include <sodium.h>
#include <cstring>

byte g_key[128];

uint64_t get_timestamp(void) {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (uint64_t) tv.tv_sec;
}

void pbytes(byte bs[], int len) {
    for (int i = 0; i < len; i++) {

        cout << std::hex << (int) bs[i] << " ";
    }
    cout << std::dec << endl;
}

void pbytess(byte bs[], int len) {
    for (int i = 0; i < len; i++) {
        cout << (char) bs[i];
    }
    cout << std::dec << endl;
}

#define check(w, r)\
    if(r<0){\
        w = false;\
        if(cok==false && sok==false){\
            break;\
        }\
    }

template <typename T>
void del_pre(T s, int n) {
    for (int i = 0; i < n; i++) {
        s[i] = s[n + i];
    }
}

void data_copy(int c, int s) {
    bool cok = true;
    bool sok = true;
#define checkbreak  if(cok == false || sok==false) break;
    int ret;
    byte buff[1024];
    fd_set fd_read;
    struct timespec time_out;
    while (true) {
        FD_ZERO(&fd_read);
        FD_SET(c, &fd_read);
        FD_SET(s, &fd_read);

        time_out.tv_sec = 1;
        time_out.tv_nsec = 0;
        ret = pselect((c > s ? c : s) + 1, &fd_read, NULL, NULL, &time_out, NULL);

        if (-1 == ret) {
            break;
        } else if (0 == ret) {
            continue;
        }

        if (FD_ISSET(c, &fd_read)) {
            ret = recv(c, buff, 1024, 0);
            if (ret > 0) {
                ret = send(s, buff, ret, 0);
                if (ret == -1) {
                    sok = false;
                    checkbreak
                }
            } else if (ret == 0) {
                cok = false;
                checkbreak
            } else {
                cok = false;
                checkbreak
            }
        } else if (FD_ISSET(s, &fd_read)) {
            ret = recv(s, buff, 1024, 0);
            if (ret > 0) {
                ret = send(c, buff, ret, 0);
                if (ret == -1) {
                    cok = false;
                    checkbreak
                }
            } else if (ret == 0) {
                sok = false;
                checkbreak
            } else {
                sok = false;
                checkbreak
            }
        }
    }
    close(c);
    close(s);
}

int send_safe(int cfd, byte *buff, int buff_size, byte en_nonce[], int &count) {
    int ret = 0;
    byte data_len[2];
    byte nonce[16];
    memcpy(nonce, en_nonce, 16);

    int2byte(nonce + 7, count, 5);

    byte des[buff_size + 16];
    int des_len = 0;


    int2byte(data_len, buff_size, 2);

    my_aesgcm256_crypt(data_len, 2, des, des_len, g_key, nonce);

    ret = send(cfd, des, des_len, 0);

    if (ret <= 0) return ret;

    count += 1;
    memcpy(nonce, en_nonce, 16);
    int2byte(nonce + 8, count, 4);

    my_aesgcm256_crypt(buff, buff_size, des, des_len, g_key, nonce);

    ret = send(cfd, des, des_len, 0);

    if (ret <= 0) return ret;
    return ret;
}

int recv_safe_send(int cfd, byte *buff, int buff_size, byte de_nonce[], int &count) {
    int ret = 0;
    byte data_len[2];
    byte buff_r[2080];
    int len_r;

    byte nonce[16];
    memcpy(nonce, de_nonce, 16);

    int2byte(nonce + 8, count, 4);


    if (my_aesgcm256_decrypt(buff, buff_size, buff_r, len_r, g_key, nonce) < 0) {
        cout << "cant decrypt a-len-data" << endl;
    } else {
        ;
    }

    send(cfd, buff_r, len_r, 0);
    count += 1;
}


int decrypt_len(byte recvs[], int len_decrypt_len, byte de_nonce[], int &recv_count) {
    byte tmp[32];
    int len;

    byte nonce[16];
    memcpy(nonce, de_nonce, 16);

    int2byte(nonce + 7, recv_count, 5);

    recv_count += 1;

    if (my_aesgcm256_decrypt(recvs, len_decrypt_len, tmp, len, g_key, nonce) < 0) {
        cout << "cant decrypt length" << endl;
        return 0;
    } else {
        int rs = (uint8_t)tmp[0] | (uint8_t)tmp[1] << 8;
        return rs + 16;
    }
}

void data_copy_safe(int c, int s, byte en_nonce[], byte de_nonce[]) {
    bool cok = true;
    bool sok = true;
    int send_count = 0;
    int recv_count = 0;

    const int REV_SIZE = 2048;
    byte recvs[3200];

    int len_recvs = 0;
    int len_sreq = 0;

#define checkbreak  if(cok == false || sok==false) break;
    int ret;
    byte buff[3200];
    fd_set fd_read;
    struct timespec time_out;
    while (true) {
        FD_ZERO(&fd_read);
        FD_SET(c, &fd_read);
        FD_SET(s, &fd_read);

        time_out.tv_sec = 1;
        time_out.tv_nsec = 0;
        ret = pselect((c > s ? c : s) + 1, &fd_read, NULL, NULL, &time_out, NULL);

        if (-1 == ret) {
            break;
        } else if (0 == ret) {
            continue;
        }

        if (FD_ISSET(c, &fd_read)) {
            ret = recv(c, buff, REV_SIZE, 0);
            if (ret > 0) {
                ret = send_safe(s, buff, ret, en_nonce, send_count);
                if (ret == -1) {
                    sok = false;
                    checkbreak
                } else {
                    send_count += 1;
                }
            } else if (ret == 0) {
                cok = false;
                checkbreak
            } else {
                cok = false;
                checkbreak
            }
        } else if (FD_ISSET(s, &fd_read)) {
            ret = recv(s, buff, REV_SIZE, 0);
            if (ret > 0) {
                byte *bufft = buff;
                deal_block:
                if (len_sreq == 0) {
                    if (ret + len_recvs >= 18) {
                        int need = 18 - len_recvs;
                        g_insert(recvs, len_recvs, bufft, need);
                        bufft += need;
                        ret -= need;
                        len_sreq = decrypt_len(recvs, 18, de_nonce, recv_count);
                    } else {
                        // wait next recive
                        continue;
                    }
                }

                if (ret + len_recvs - 18 >= len_sreq) {
                    int need = len_sreq - (len_recvs - 18);
                    g_insert(recvs, len_recvs, bufft, need);
                    recv_safe_send(c, recvs + 18, len_sreq, de_nonce, recv_count);
                    len_recvs = 0;
                    len_sreq = 0;
                    bufft += need;
                    ret -= need;
                    goto deal_block;
                }else{
                    g_insert(recvs, len_recvs, bufft, ret);
                }
            } else if (ret == 0) {
                sok = false;
                checkbreak
            } else {
                sok = false;
                checkbreak
            }
        }
    }
    close(c);
    close(s);
}

int getopt_from(int &port, char *key, int &is_client, int argc, char *argv[])
{
    int opt;

    while((opt = getopt(argc, argv, "scp:k:")) != -1) {
        switch (opt) {
            case 's':
                is_client |= 2 ;
                break;
            case 'c':
                is_client |= 1 ;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'k':
                strcpy(key,optarg);
                break;
            default:
                fprintf(stderr, "%c usage: %s -[s|c][-p port] [-k key]\n", opt,argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    return 0;
}