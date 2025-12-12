#include "heartbeat.h"
#include "colors.h"
#include <iostream>
#include <pthread.h>
#include <arpa/inet.h>
#include <cstring>
#include "data_transfer/socket_utils.h"

#define MAXLINE_HEARTBEAT 32

namespace election {

    void heartbeat_receiver(int port){
        int sockfd;
        char buffer[MAXLINE_HEARTBEAT + 1];
        char send_buffer[MAXLINE_HEARTBEAT + 1];
        struct sockaddr_in servaddr, cliaddr;

        // Creating socket file descriptor
        sockfd = create_udp_socket();

        // memset(&servaddr, 0, sizeof(servaddr));
        memset(&cliaddr, 0, sizeof(cliaddr));

        // Filling server information
        servaddr = create_sockaddr("0.0.0.0", port);

        // Bind the socket with the server address
        bind_to_sockaddr(sockfd, &servaddr);

        socklen_t len;
        int n;

        len = sizeof(cliaddr);

        std::string msg;

        // Generate response message
        snprintf(send_buffer, sizeof(send_buffer), "ANS");

        while (true) {
            // receive incoming heartbeat requests
            n = recvfrom(sockfd, (char *)buffer, MAXLINE_HEARTBEAT, MSG_WAITALL,
                        (struct sockaddr *)&cliaddr, &len);
            buffer[n] = '\0';

            // answer with a heartbeat response
            sendto(sockfd, send_buffer, strlen(send_buffer), MSG_CONFIRM,
                (const struct sockaddr *)&cliaddr, len);

    #if _DEBUG
            std::cout << GREEN
                    << "Heartbeat request from: " << inet_ntoa(cliaddr.sin_addr)
                    << ":" << ntohs(cliaddr.sin_port) << RESET << std::endl;
    #endif
        }
    }



    void *start_heartbeat_receiver(void *arg) {
        int port = (*(int *)arg) + 2;

    #if _DEBUG
        std::cout << MAGENTA << "Starting Heartbeat Receiver on port " << port << RESET
                << std::endl;
    #endif
        (void)heartbeat_receiver(port);
        return 0;
}

} // namespace election