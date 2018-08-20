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
#include <csignal>

using std::byte;
using std::cout;
using std::cin;
using std::endl;

void deal_simage(int c) {
    byte buff[1024];
    byte des[1024];
    int cur = 0, des_len = 0;

    require_n(c, buff, 60);

    byte en_nonce[16];
    byte de_nonce[16];

    crypto_generichash((unsigned char *) en_nonce, 16,
                       (unsigned char *) (buff+24), 16,
                       nullptr, 0);

    crypto_generichash((unsigned char *) de_nonce, 16,
                       (unsigned char *) (buff+32), 16,
                       nullptr, 0);

    if(my_aesgcm256_decrypt(buff+16, 44, des, des_len, g_key, buff)<0){
        cout<<"A bad request, cant decrypt data !"<<endl;
        pbytes(buff+16,44);
        pbytes(buff,12);
        pbytes(g_key,32);
    }else{
        //cout<<"A new request accept!"<<endl;
        uint64_t t = 0;
        byte2int(des+16,t,8);
        byte nonce[16];
        cur=0;
        g_insert(nonce,cur,buff,16);

        nonce[8]=des[24];
        nonce[9]=des[25];
        nonce[10]=des[26];
        nonce[11]=des[27];
        require_n(c, buff, 25);
        if(my_aesgcm256_decrypt(buff, 25, des, des_len, g_key, nonce)<0){
            cout<<"A bad request, cant decrypt data[cmd] !"<<endl;
        }else{
            size_t addr_len = size_t(des[7]);
            size_t rand2_len = size_t(des[8]);
            nonce[11] = byte(int(nonce[11]) + 1);

            int port = ((int)des[4])<<8 | int(des[5]);

            require_n(c, buff, addr_len+16);

            if(my_aesgcm256_decrypt(buff, addr_len+16, des, des_len, g_key, nonce)<0){
                cout<<"A bad request, cant decrypt data[addr] !"<<addr_len<<endl;
            }else{
                require_n(c, buff, rand2_len);

                des[des_len] = byte(0);

                char ip_addr[512];
                struct addrinfo *res = {};
                struct addrinfo hint = {};
                hint.ai_family = AF_UNSPEC;
                if(getaddrinfo((char *) des, NULL, &hint, &res)!=0){
                    cout<<"cant resolve name: "<<(char *) des<<endl;
                    return;
                }

                struct sockaddr_in *in;
                in = (struct sockaddr_in *) (res->ai_addr);
                inet_ntop(AF_INET, &in->sin_addr, ip_addr, sizeof(ip_addr));

                struct sockaddr_in dest_addr;
                int sockfd = socket(AF_INET, SOCK_STREAM, 0);
                dest_addr.sin_family = AF_INET;
                dest_addr.sin_port = htons(port);
                dest_addr.sin_addr.s_addr = inet_addr(ip_addr);
                freeaddrinfo(res);
                cout<<"will connected to "<<(char *)des<<"#"<<ip_addr<<":"<<port<<endl;
                if(connect(sockfd, (struct sockaddr *) &dest_addr, sizeof(struct sockaddr))<0){
                    cout<<"CANT connected to "<<(char *)des<<"#"<<ip_addr<<":"<<port<<endl;
                }
                data_copy_safe(sockfd,c,en_nonce,de_nonce);
            }
        }
    }
}


int main_server(int port) {
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);
    int tmp=1;
    setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&tmp,sizeof(tmp));
    if(bind(lfd, (struct sockaddr *) &sin, sizeof(struct sockaddr_in))<0){
        cout<<"Cant bind to port "<<port<<endl;
        exit(-1);
    }
    if(listen(lfd, 32) < 0){
        cout<<"Cant listen to port "<<port<<endl;
        exit(-1);
    }
    bool stop_accept = false;
    cout<<"simage server, listen port "<<port<<endl;
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
    return 0;
}