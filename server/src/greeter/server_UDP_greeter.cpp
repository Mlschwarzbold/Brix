// Server side implementation of UDP client-server model
#include "server_UDP_greeter.h"
#include "server_discovery_service.h"
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 8080
#define MAXLINE 1024

namespace udp_server_greeter {

int start_server() {

    udp_server_greeter::server_discovery_service(4000, (char *)"217.0.0.1",
                                                 4001);
    int sockfd;
    char buffer[MAXLINE + 1];
    char send_buffer[MAXLINE + 1];
    struct sockaddr_in servaddr, cliaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Enable broadcast
    int broadcastEnable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable,
                   sizeof(broadcastEnable)) < 0) {
        perror("setsockopt (SO_BROADCAST) failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // binding udp socket to subnet broadcast address
    // servaddr.sin_addr.s_addr = inet_addr("192.168.0.255");

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) <
        0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    socklen_t len;
    int n;

    len = sizeof(cliaddr); // len is value/result

    std::string msg;

    while (true) {
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                     (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';
        printf("Client>>  %s\n", buffer);

        // get message from stdin
        std::cout << "Escreve uma resposta ";
        std::getline(std::cin, msg);

        // copy to send buffer
        strncpy(send_buffer, msg.c_str(), MAXLINE);

        sendto(sockfd, send_buffer, strlen(send_buffer), MSG_CONFIRM,
               (const struct sockaddr *)&cliaddr, len);
        std::cout << ">> " << send_buffer << std::endl;
    }

    /*
    n = recvfrom(sockfd, (char *)buffer, MAXLINE,
    MSG_WAITALL, ( struct sockaddr *) &cliaddr,
    &len);
    buffer[n] = '\0';
    printf("Client : %s\n", buffer);
    sendto(sockfd, (const char *)hello, strlen(hello),
    MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
    len);
    std::cout<<"Hello message sent."<<std::endl;

    */

    return 0;
}
} // namespace udp_server_greeter