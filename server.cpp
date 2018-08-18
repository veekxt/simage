#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    struct sockaddr_in dest_addr; /* 目的地址*/
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); /* 错误检查 */
    dest_addr.sin_family = AF_INET; /* host byte order */
    dest_addr.sin_port = htons(44228); /* short, network byte order */
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* don't forget to error check the connect()! */

    connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
    
    send(sockfd,"hello",4,0);
    
    return 0;
}