#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <unordered_map>
#include <bits/stdc++.h>
#include <thread>
#include "packet_multiplexer.h"
#include "data_transfer/socket_utils.h"
#include "colors.h"
#include "packet_indexer.h"
#include "packets/packets.h"
#include "packets/string_packets.h"
#include "request_handler/REQ_packet_handler.h"

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
    Indexing_result result;
    Packet_indexer indexer = Packet_indexer();
    


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

        // convert to string_packet
        String_Packet str_packet(buffer);
        Packet_type packet_type = str_packet.type();

        
        // If it is a REQ packet, try to parse it
        if(packet_type == REQ){
            try {
                REQ_Packet req_packet = str_packet.to_REQ_Packet();
                std::cout << GREEN << "REQ Packet - Seq Num: " << req_packet.seq_num 
                    << " Receiver IP: " << inet_ntoa(*(in_addr*)&req_packet.receiver_ip)
                    << " Transfer Amount: " << req_packet.transfer_amount << RESET << std::endl;

                result = indexer.index_packet(req_packet, (in_addr_t)cliaddr.sin_addr.s_addr);
                print_status(result);

                // Having found whether the packet is valid, duplicate or out of order,
                // we create a thread to handle it.
                // The indexing of incoming packets is done in the same thread as the multiplexer
                // because it would be more complex to do it in a dedicated thread per packet
                // as the newly spawned thread would not know what the seq number was supposed to be
                // This could be solved having persistent threads to handle each client
                // a.k.a having a connection oriented approach
                // that would require more complex thread communication and would probably introduce busy wait


                std::thread request_handler_thread(requests::route_REQ_packet, req_packet,
                     (in_addr_t)cliaddr.sin_addr.s_addr,
                        result.last_info
                    );
                request_handler_thread.detach();

            } catch (const std::exception& e) {
                std::cerr << RED << "Error parsing REQ Packet: " << e.what() << RESET << std::endl;
            }
            

        } else {
            std::cout << RED << "Unexpected Packet Type" << RESET << std::endl;
        }

        
    }

    

    return 0;
}

} // namespace multiplexer