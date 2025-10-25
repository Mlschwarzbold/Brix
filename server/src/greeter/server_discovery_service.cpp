#include "colors.h"
#include "data_transfer/socket_utils.h"
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "db_manager/db_manager.h"

namespace udp_server_greeter {

const int MAXLINE = 2048;

// Server Discovery Service
// This function implements a UDP server that listens for discovery requests
// from clients When a discovery request is received, it responds with the
// requests server's IP and port
void server_discovery_service(int discovery_service_port,
                              char *requests_server_ip,
                              int requests_server_port) {
    int sockfd;
    char buffer[MAXLINE + 1];
    char send_buffer[MAXLINE + 1];
    struct sockaddr_in servaddr, cliaddr;

    std::cout << GREEN << "Starting Discovery Service on port "
              << discovery_service_port << RESET << std::endl;

    auto db_instance = db_manager::DbManager::get_instance();

    // Creating socket file descriptor
    sockfd = create_udp_socket();

    // Enable broadcast
    enable_broadcast(sockfd);

    // memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr = create_sockaddr("0.0.0.0", discovery_service_port);

    // Bind the socket with the server address
    bind_to_sockaddr(sockfd, &servaddr);

    socklen_t len;
    int n;

    len = sizeof(cliaddr);

    std::string msg;

    // Generate response message
    snprintf(send_buffer, sizeof(send_buffer), "LOC %s %d END",
             requests_server_ip, requests_server_port);

    while (true) {
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                     (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';

        db_instance->register_client(cliaddr.sin_addr.s_addr);

        sendto(sockfd, send_buffer, strlen(send_buffer), MSG_CONFIRM,
               (const struct sockaddr *)&cliaddr, len);

        std::cout << GREEN
                  << "Discovery request from: " << inet_ntoa(cliaddr.sin_addr)
                  << ":" << ntohs(cliaddr.sin_port) << RESET << std::endl;
    }
}

} // namespace udp_server_greeter