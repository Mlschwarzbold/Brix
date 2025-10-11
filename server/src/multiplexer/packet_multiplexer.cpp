#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <unordered_map>
#include <bits/stdc++.h>
#include "packet_multiplexer.h"
#include "data_transfer/socket_utils.h"
#include "colors.h"
#include "packet_indexer.h"
#include "packets/packets.h"
#include "packets/string_packets.h"

#define MAX_PACKET_SIZE 1024



namespace multiplexer {

int start_multiplexer_server(int port){

    std::cout << BLUE <<  "Starting Packet Multiplexer on port " << port << RESET<< std::endl;
    (void) packet_multiplexer(port);
    return 0;
}

int packet_multiplexer(int port){

    int sockfd;
    int n;
    socklen_t len;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[MAX_PACKET_SIZE+1]; //+1 for /0
    //char send_buffer[MAXLINE+1];
    


    // Creating socket file descriptor
    sockfd = create_udp_socket();

    //memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    len = sizeof(cliaddr); 

    // Filling server information
    servaddr = create_sockaddr("0.0.0.0", port);

    // Bind the socket with the server address
    bind_to_sockaddr(sockfd, &servaddr);

    

    while (true) {
        // wait for udp datagrams
        n = recvfrom(sockfd, (char *)buffer, MAX_PACKET_SIZE, MSG_WAITALL,
                        (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';

        std::cout << BLUE << "Packet from: " << inet_ntoa(cliaddr.sin_addr)
                    << ":" << ntohs(cliaddr.sin_port) << RESET <<std::endl;
        std::cout << BLUE << "Packet content: " << buffer << RESET << std::endl;

        
    }

    // convert to string_packet
    String_Packet str_packet(buffer);
    Packet_type packet_type = str_packet.type();

    

    switch (packet_type) {
        case REQ:

            
            break;
        case ANS:
            
            break;
        default:
            std::cerr << RED << "Unknown packet type" << RESET << std::endl;
            break;
    }

    return 0;
}

} // namespace multiplexer