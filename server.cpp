#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstddef>
#include <thread>
#include <unistd.h>
#include "crypt.h"
#include "utils.h"
#include <sodium.h>
#include <netdb.h>

using std::byte;
using std::cout;
using std::cin;
using std::endl;

void deal_simage(int c) {
    int ret;
    byte buff[1024];
    byte des[1024];
    int cur = 0, des_len = 0;
    fd_set fd_read;
    struct timeval time_out = {0, 1000000};
    recv(c, buff, 60, 0);
    byte key[32];
    crypto_generichash((unsigned char *) key, 32,
                       (unsigned char *) "for test string", 15,
                       nullptr, 0);

    if(my_aesgcm256_decrypt(buff+16, 44, des, des_len, key, buff)<0){
        cout<<"A bad request, cant decrypt data !"<<endl;
    }else{
        cout<<"A new request accept!"<<endl;
        uint64_t t = 0;
        byte_int64(des+16,t);
        byte user[16];
        crypto_generichash((unsigned char *) user, 16,
                           (unsigned char *) "for test string", 15,
                           nullptr, 0);

        byte nonce[12];
        cur=0;
        g_insert(nonce,cur,buff,16);

        nonce[8]=des[24];
        nonce[9]=des[25];
        nonce[10]=des[26];
        nonce[11]=des[27];

        recv(c, buff, 24, 0);

        if(my_aesgcm256_decrypt(buff, 24, des, des_len, key, nonce)<0){
            cout<<"A bad request, cant decrypt data[cmd] !"<<endl;
        }else{
            size_t addr_len = size_t(des[7]);
            nonce[11] = byte(int(nonce[11]) + 1);

            int port = ((int)des[4])<<8 | int(des[5]);

            recv(c, buff, addr_len+16, 0);
            if(my_aesgcm256_decrypt(buff, addr_len+16, des, des_len, key, nonce)<0){
                cout<<"A bad request, cant decrypt data[addr] !"<<addr_len<<endl;
            }else{
                des[des_len] = byte(0);

                char ip_addr[1024];
                struct addrinfo *res = {};
                struct addrinfo hint = {};
                hint.ai_family = AF_UNSPEC;
                getaddrinfo((char *) des, NULL, &hint, &res);

                struct sockaddr_in *in;
                in = (struct sockaddr_in *) (res->ai_addr);
                inet_ntop(AF_INET, &in->sin_addr, ip_addr, sizeof(ip_addr));

                struct sockaddr_in dest_addr;
                int sockfd = socket(AF_INET, SOCK_STREAM, 0);
                dest_addr.sin_family = AF_INET;
                dest_addr.sin_port = htons(port);
                dest_addr.sin_addr.s_addr = inet_addr(ip_addr);
                connect(sockfd, (struct sockaddr *) &dest_addr, sizeof(struct sockaddr));
                data_copy(c, sockfd);
            }
        }
    }
}

void init_simage() {
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(44229);
    bind(lfd, (struct sockaddr *) &sin, sizeof(struct sockaddr_in));
    listen(lfd, 32);
    bool stop_accept = false;
    while (!stop_accept) {
        struct sockaddr_in cin;
        socklen_t len = sizeof(cin);
        int cfd = accept(lfd, (struct sockaddr *) &cin, &len);
        if (cfd < 0) {
            cout << "accept error!";
        } else {
            std::thread t(deal_simage, cfd);
            t.detach();
        }
    }
}

int main() {
    sodium_init();
    init_simage();
    return 0;
}