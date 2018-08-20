#include <iostream>
#include <csignal>
#include <sodium.h>
#include <thread>
#include <execinfo.h>
#include "simage.h"
#include "utils.h"

int main(int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);

    if (sodium_init() < 0) {
        cout << "cant init sodium!" << endl;
        exit(-1);
    }

    int port = 0, is_client = 0;

    unsigned char key[128] = {};
    unsigned char target_all[128] = {};

    char target[128];
    int port_t;


    getopt_from(port, (char *) key, is_client, (char *) target_all, argc, argv);

    sscanf(reinterpret_cast<const char *>(target_all), "%[^:]:%d", target,&port_t);

    crypto_generichash((unsigned char *) g_key, 16,
                       (unsigned char *) key, 15,
                       nullptr, 0);

    if (is_client & 1 && is_client & 2) {
        cout << "You start server and client on same localhost !" << endl;
        cout << "This just for test, check your args !!!" << endl;

        std::thread t1(main_client, port, port_t, (char *)target);
        usleep(1000);
        std::thread t2(main_server, port + 1);
        t1.join();
        t2.join();
    }
    if (is_client & 1) {
        std::thread t1(main_client, port, port_t, (char *)target);
        t1.join();
    } else if (is_client & 2) {
        std::thread t1(main_server, port);
        t1.join();
    }

    return 0;
}