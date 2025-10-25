// Client side implementation of UDP client-server model
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

const int PORT = 5000;
const int MAXLINE = 2048;

int client_UDP_broadcast() {
    int sockfd;
    char buffer[MAXLINE + 1];
    char send_buffer[MAXLINE + 1];
    struct sockaddr_in servaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    const int broadcastEnable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable,
                   sizeof(broadcastEnable)) < 0) {
        perror("setsockopt (SO_BROADCAST) failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    // servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_addr.s_addr =
        inet_addr("255.255.255.255"); // this is my server IP address

    int n;
    socklen_t len;
    std::string msg;

    while (true) {
        // get message from stdin
        std::cout << "Escreve algo ai (ou 'exit' pra fechar o cliente): ";
        std::getline(std::cin, msg);
        if (msg == "exit") {
            break;
        }
        // copy to send buffer
        strncpy(send_buffer, msg.c_str(), MAXLINE);
        sendto(sockfd, send_buffer, strlen(send_buffer), MSG_CONFIRM,
               (const struct sockaddr *)&servaddr, sizeof(servaddr));
        std::cout << " >> " << send_buffer << std::endl;

        std::cout << "Escreve algo ai (ou 'exit' pra fechar o cliente): ";
        std::getline(std::cin, msg);
        if (msg == "exit") {
            break;
        }
        // copy to send buffer
        strncpy(send_buffer, msg.c_str(), MAXLINE);
        sendto(sockfd, send_buffer, strlen(send_buffer), MSG_CONFIRM,
               (const struct sockaddr *)&servaddr, sizeof(servaddr));
        std::cout << " >> " << send_buffer << std::endl;

        // espera receber mensagem

        // len = sizeof(servaddr); // is copilot making stuff up
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                     (struct sockaddr *)&servaddr, &len);
        buffer[n] = '\0';
        std::cout << "Server>> : " << buffer << std::endl;
    }

    /*
    sendto(sockfd, (const char *)hello, strlen(hello),
        MSG_CONFIRM, (const struct sockaddr *) &servaddr,
            sizeof(servaddr));
    std::cout<<"Hello message sent."<<std::endl;


    n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                MSG_WAITALL, (struct sockaddr *) &servaddr,
                &len);
    buffer[n] = '\0';
    std::cout<<"Server :"<<buffer<<std::endl;
    */
    close(sockfd);
    return 0;
}