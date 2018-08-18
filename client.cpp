#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstddef>
#include <thread>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <ctime>
#include <csignal>
#include "crypt.h"
#include "utils.h"
#include <vector>

using std::byte;
using std::vector;
using std::cout;
using std::cin;
using std::endl;


void connect_remote(int cfd, int cmd, int port, int addr_type, int addr_len, char *addr) {
    struct sockaddr_in dest_addr; /* 目的地址*/
    int sfd = socket(AF_INET, SOCK_STREAM, 0); /* 错误检查 */
    dest_addr.sin_family = AF_INET; /* host byte order */
    dest_addr.sin_port = htons(44229); /* short, network byte order */
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    connect(sfd, (struct sockaddr *) &dest_addr, sizeof(struct sockaddr));

    const int NONCE_LEN = 16;
    const int USER_LEN = 16;
    const int NONCE_REAL_LEN = 12;
    const int KEY_LEN = 32;

    byte buff[1024];
    byte des[1024];
    int des_len;
    int cur = 0;

    byte nonce[NONCE_LEN];
    randombytes_buf(nonce, NONCE_LEN);
    send(sfd, nonce, NONCE_LEN, 0);

    byte key[KEY_LEN];
    byte user[USER_LEN];
    byte timestamp[8];
    byte rand[4];

    crypto_generichash((unsigned char *) key, KEY_LEN,
                       (unsigned char *) "for test string", 15,
                       nullptr, 0);

    crypto_generichash((unsigned char *) user, USER_LEN,
                       (unsigned char *) "for test string", 15,
                       nullptr, 0);
    g_insert(buff, cur, user, USER_LEN);

    uint64_t t = get_timestamp();
    int64_byte(timestamp, t);

    g_insert(buff, cur, timestamp, 8);

    randombytes_buf(rand, 4);
    g_insert(buff, cur, rand, 4);

    my_aesgcm256_crypt(buff, cur, des, des_len, key, nonce);

    send(sfd, des, des_len, 0);

    buff[0] = byte(1);
    buff[1] = byte(0);
    buff[2] = byte(0);
    buff[3] = byte(cmd);
    buff[4] = byte(port >> 8);
    buff[5] = byte(port);
    buff[6] = byte(addr_type);
    buff[7] = byte(addr_len);
    cur = 8;
    g_insert(buff, cur, addr, addr_len);

    buff[cur] = byte(0);

    nonce[8] = rand[0];
    nonce[9] = rand[1];
    nonce[10] = rand[2];
    nonce[11] = rand[3];

    my_aesgcm256_crypt(buff, 8, des, des_len, key, nonce);
    send(sfd, des, des_len, 0);

    nonce[11] = byte(int(nonce[11]) + 1);

    my_aesgcm256_crypt(buff+8, addr_len, des, des_len, key, nonce);

    send(sfd,des,des_len,0);

    data_copy(cfd,sfd);
}

void deal_socks5(int cfd) {
    byte buff[8192];
    ssize_t done = recv(cfd, buff, 3, 0);
    ssize_t send_done;
    if (done <= 0) {
        cout << "rev error, exit thread!\n";
        close(cfd);
        return;
    } else {
        buff[0] = (byte) 0x05;
        buff[1] = (byte) 0x00;
        send_done = send(cfd, buff, 2, 0);
        if (send_done <= 0) {
            cout << "rev error, exit thread!\n";
            close(cfd);
            return;
        }
    }

    done = recv(cfd, buff, 4, 0);
    if (done <= 0) {
        cout << "rev error, exit thread!\n";
        close(cfd);
        return;
    } else {
        int cmd = int(buff[1]);
        int hand = 0;
        int addr_type = (int) buff[3];

        // todo switch
        done = recv(cfd, buff + hand, 1, 0);
        if (done <= 0) {
            cout << "rev error, exit thread!\n";
            close(cfd);
            return;
        }
        size_t addr_len = (size_t) (buff + hand)[0];
        hand += done;

        done = recv(cfd, buff + hand, addr_len, 0);
        if (done <= 0) {
            cout << "rev error, exit thread!\n";
            close(cfd);
            return;
        }
        (buff + hand)[addr_len] = (byte) 0;
        byte addr[128];
        int tmp_c = 0 ;
        g_insert(addr,tmp_c,(buff + hand),addr_len);

        hand += done;

        done = recv(cfd, (buff + hand), 2, 0);
        if (done <= 0) {
            cout << "rev error, exit thread!\n";
            close(cfd);
            return;
        }
        int port = (int) (buff + hand)[1] | (int) (buff + hand)[0] << 8;
        hand += done;

        byte tmp[4] = {byte(5), byte(0), byte(0), byte(3)};

        send_done = send(cfd, tmp, 4, 0);
        if (send_done <= 0) {
            cout << "rev error, exit thread!\n";
            close(cfd);
            return;
        }
        send_done = send(cfd, buff, hand, 0);
        if (send_done <= 0) {
            cout << "rev error, exit thread!\n";
            close(cfd);
            return;
        }
        connect_remote(cfd, cmd, port, addr_type, addr_len, (char *)addr);
    }
}

void init_socks5() {
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(44228);
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
            std::thread t(deal_socks5, cfd);
            t.detach();
        }
    }
}

int main() {
    signal(SIGPIPE, SIG_IGN);
    sodium_init();
    if (crypto_aead_aes256gcm_is_available() == 0) {
        abort();
    }
    cout << "Start Client> port 44228" << endl;
    init_socks5();
    return 0;
}