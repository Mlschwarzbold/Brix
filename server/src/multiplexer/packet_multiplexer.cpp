#include "packet_multiplexer.h"
#include "colors.h"
#include "data_transfer/socket_utils.h"
#include "packets/packets.h"
#include "packets/string_packets.h"
#include "request_handler/KIL_packet_handler.h"
#include "request_handler/REQ_packet_handler.h"
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <iostream>
#include <string>

#define MAX_PACKET_SIZE 1024

namespace multiplexer {

void *start_multiplexer_server(void *arg) {
    int port = (*(int *)arg) + 1;

    std::cout << BLUE << "Starting Packet Multiplexer on port " << port << RESET
              << std::endl;
    (void)packet_multiplexer(port);
    return 0;
}

int packet_multiplexer(int port) {

    int sockfd;
    int n;
    socklen_t len;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[MAX_PACKET_SIZE + 1]; //+1 for /0

    // Creating socket file descriptor
    sockfd = create_udp_socket();

    // memset(&servaddr, 0, sizeof(servaddr));
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
                  << ":" << ntohs(cliaddr.sin_port) << RESET << std::endl;
        std::cout << BLUE << "Packet content: " << buffer << RESET << std::endl;

        // convert to string_packet
        String_Packet str_packet(buffer);
        Packet_type packet_type = str_packet.type();

        // If it is a REQ packet, try to parse it
        if (packet_type == REQ) {
            try {
                REQ_Packet req_packet = str_packet.to_REQ_Packet();

                pthread_t req_thread;

                requests::process_req_packet_params params = {
                    cliaddr, req_packet, sockfd};

                requests::process_req_packet_params params_copy = params;

                pthread_create(&req_thread, NULL, requests::process_req_packet,
                               &params_copy);

            } catch (const std::exception &e) {
                std::cerr << RED << "Error parsing REQ Packet: " << e.what()
                          << RESET << std::endl;
            }

        } else if (packet_type == KIL) {
            KIL_Packet kill_packet = str_packet.to_KIL_Packet();

            pthread_t kill_thread;

            requests::process_kill_packet_params params = {cliaddr, kill_packet,
                                                           sockfd};

            pthread_create(&kill_thread, NULL, requests::process_kill_packet,
                           &params);
        } else {
            std::cout << RED << "Unexpected Packet Type" << RESET << std::endl;
        }
    }

    return 0;
}

} // namespace multiplexer