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

using std::byte;
using std::cout;
using std::cin;
using std::endl;

int num = 0;

void pbytes(byte bs[], int len) {
    for (int i = 0; i < len; i++) {

        cout << std::hex << (int) bs[i] << " ";
    }
    cout << endl;
}

void pbytess(byte bs[], int len) {
    for (int i = 0; i < len; i++) {
        cout << (char) bs[i];
    }
    cout << endl;
}

void data_copy(int c, int s) {
    int done = 0;
    int ret;
    byte buff[1024];
    fd_set fd_read;
    struct timeval time_out={0, 1000000};
    while (true) {
        FD_ZERO(&fd_read);
        FD_SET(c, &fd_read);
        FD_SET(s, &fd_read);

        time_out={0, 1000000};
        ret = select((c > s ? c : s) + 1, &fd_read, NULL, NULL, &time_out);

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
                    break;
                }
            } else if (ret == 0) {
                break;
            } else {
                break;
            }
        } else if (FD_ISSET(s, &fd_read)) {
            ret = recv(s, buff, 1024, 0);
            if (ret > 0) {
                ret = send(c, buff, ret, 0);
                if (ret == -1) {
                    break;
                }
            } else if (ret == 0) {
                break;
            } else {
                break;
            }
        }
    }
    close(c);
    close(s);
}

void deal_socks5(int cfd) {
    byte buff[8192];
    char ip_addr[128];
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
        int hand = 0;
        size_t addr_type = (size_t) buff[3];

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
        struct addrinfo *res = {};
        struct addrinfo hint = {};
        hint.ai_family = AF_UNSPEC;
        getaddrinfo((char *) buff + hand, NULL, &hint, &res);

        struct sockaddr_in *in;

        in = (struct sockaddr_in *) (res->ai_addr);
        //int port = ntohs(in->sin_port);
        inet_ntop(AF_INET, &in->sin_addr, ip_addr, sizeof(ip_addr));
        hand += done;

        done = recv(cfd, (buff + hand), 2, 0);
        if (done <= 0) {
            cout << "rev error, exit thread!\n";
            close(cfd);
            return;
        }
        int port = (int) (buff + hand)[1] | (int) (buff + hand)[0] << 8;

        struct sockaddr_in dest_addr; /* 目的地址*/
        int sockfd = socket(AF_INET, SOCK_STREAM, 0); /* 错误检查 */
        dest_addr.sin_family = AF_INET; /* host byte order */
        dest_addr.sin_port = htons(port); /* short, network byte order */
        dest_addr.sin_addr.s_addr = inet_addr(ip_addr);

        hand += done;

        connect(sockfd, (struct sockaddr *) &dest_addr, sizeof(struct sockaddr));

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
        //fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
        //fcntl(cfd, F_SETFL, fcntl(cfd, F_GETFL, 0) | O_NONBLOCK);
        freeaddrinfo(res);
        data_copy(cfd, sockfd);
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
    init_socks5();
    return 0;
}