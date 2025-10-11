#include <arpa/inet.h>
#include <unordered_map>
#include "packet_indexer.h"
#include <string>
#include <iostream>

namespace multiplexer {

    Packet_indexer::Packet_indexer() {
        client_packet_table = std::unordered_map<in_addr_t, Last_packet_info>();
    }

    Packet_status Packet_indexer::index_packet(REQ_Packet packet, in_addr_t client_ip){ 
        std::cout << "Indexing packet with seq num: " << packet.seq_num << " from client: " << inet_ntoa(*(in_addr*)&client_ip) << std::endl;

        auto it = client_packet_table.find(client_ip);

        // Check if the client IP is already in the table
        if (it != client_packet_table.end()) {
            int last_seq_num = it->second.last_seq_num;
            // If client has already sent packets, check sequence number
            if (packet.seq_num == last_seq_num + 1) {
                // If seq num is correct, update the table and return 1 (valid packet)
                it->second.last_seq_num = packet.seq_num;
                return VALID; 
            } else {
                if (packet.seq_num <= last_seq_num) {
                    return DUPLICATE;
                } else {
                    return OUT_OF_ORDER;
                }
            }
        } else {
            // New client, check if seq num is 1
            std::cout << "New client detected: " << inet_ntoa(*(in_addr*)&client_ip) << std::endl;
            if (packet.seq_num == 1) {
                client_packet_table.insert({client_ip, {packet.seq_num, false, nullptr}});
                return VALID;
            } else {
                return OUT_OF_ORDER;
            }
        }


       return NO_CLUE;
    }

} // namespace multiplexer
