#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string>

int create_udp_socket();

int enable_broadcast(int sockfd);

int set_timeout(int sockfd, int timeout_ms);

struct sockaddr_in create_sockaddr(const char *ip, int port);

int bind_to_sockaddr(int sockfd, struct sockaddr_in *addr);

void close_socket(int sockfd);

std::string addr_to_string(in_addr_t addr);

#endif // SOCKET_UTILS_H