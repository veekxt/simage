
#include <sys/time.h>
#include <sys/socket.h>
#include "utils.h"

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

    static int num=0;

void data_copy(int c, int s) {
    bool cok = true;
    bool sok = true;
    cout<<"all socks"<<num<<endl;
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
        ret = pselect((c > s ? c : s) + 1, &fd_read, NULL, NULL, &time_out,NULL);

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