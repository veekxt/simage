#include <iostream>
#include <csignal>
#include <sodium.h>
#include <thread>
#include "simage.h"
#include "utils.h"

int main(int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);
    sodium_init();

    int port = 0, is_client = 0;

    unsigned char key[128];

    getopt_from(port, (char *) key, is_client, argc, argv);

    crypto_generichash((unsigned char *) g_key, 16,
                       (unsigned char *) key, 15,
                       nullptr, 0);

    if(is_client & 1 && is_client & 2){
        cout<<"You start server and client on same localhost !"<<endl;
        cout<<"This just for test, check your args !!!"<<endl;
        std::thread t1(main_client, port);
        usleep(1000);
        std::thread t2(main_server, port + 1);
        t1.join();
        t2.join();
    }
/*
 *  useless, i am testing
 *
    if (is_client & 1) {
        std::thread t1(main_client, port);
        t1.join();
    }else if (is_client & 2) {
        std::thread t1(main_server, port);
        t1.join();
    }
*/
    return 0;
}