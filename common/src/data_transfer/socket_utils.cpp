#include "socket_utils.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

// Create a UDP socket and return its file descriptor
int create_udp_socket(){
    int sockfd;
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "socket creation failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    return sockfd;
}


// Enable broadcasting on the given socket
int enable_broadcast(int sockfd){
    int broadcastEnable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable,
                   sizeof(broadcastEnable)) < 0) {
        std::cerr << "setsockopt (SO_BROADCAST) failed" << std::endl;
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    return 0;
}

// Set a timeout on the given socket in milliseconds
// if the timeout is reached, recvfrom will return -1
int set_timeout(int sockfd, int timeout_ms){
    // weird struct takes seconds and microseconds.
    // microseconds cant be greater than 1 million
    // so we gotta do weird math to convert milliseconds to both seconds and
    // microseconds
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000; // microseconds
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        std::cerr << "setsockopt (SO_RCVTIMEO) failed" << std::endl;
        close(sockfd);
        return -1;
    }
    return 0;
}

struct sockaddr_in create_sockaddr(const char* ip, int port){
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; // IPv4
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    return addr;
}

int bind_to_sockaddr(int sockfd, struct sockaddr_in* addr){
  if (bind(sockfd,(const struct sockaddr *)addr, sizeof(*addr)) <
        0) {
        std::cerr << "bind failed" << std::endl;
        close(sockfd);
        return -1;
    }
    return 0;
}
