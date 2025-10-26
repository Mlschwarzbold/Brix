#include <arpa/inet.h>
#include <unordered_map>
#include "packet_indexer.h"
#include <string>
#include <iostream>
#include "colors.h"

namespace multiplexer {

    Packet_indexer::Packet_indexer() {
        client_packet_table = std::unordered_map<in_addr_t, Last_packet_info>();
    }

    Indexing_result Packet_indexer::index_packet(REQ_Packet packet, in_addr_t client_ip){ 
        std::cout << "Indexing packet with seq num: " << packet.seq_num << " from client: " << inet_ntoa(*(in_addr*)&client_ip) << std::endl;

        auto it = client_packet_table.find(client_ip);

        // Check if the client IP is already in the table
        if (it != client_packet_table.end()) {
            int last_seq_num = it->second.last_seq_num;
            // If client has already sent packets, check sequence number
            if (packet.seq_num == last_seq_num + 1) {
                // If seq num is correct, update the table and return 1 (valid packet)
                it->second.last_seq_num = packet.seq_num;
                return {VALID, &it->second};
            } else {
                if (packet.seq_num <= last_seq_num) {
                    return {DUPLICATE, &it->second};
                } else {
                    return {OUT_OF_ORDER, &it->second}; // should never happen with stop-and-wait
                }
            }
        } else {
            // New client, check if seq num is 1
            std::cout << "New client detected: " << inet_ntoa(*(in_addr*)&client_ip) << std::endl;
            if (packet.seq_num == 1) {

                client_packet_table.insert({client_ip, {packet.seq_num, false, nullptr}});
                return {VALID, &client_packet_table[client_ip]};
            } else {
                return {OUT_OF_ORDER, nullptr}; // should never happen
            }
        }


       return {NO_CLUE, nullptr};
    }

    void print_status(Indexing_result result) {
        std::cout << GREEN << "Packet status: ";
        switch(result.status){
                    case VALID:
                        std::cout << "VALID" << RESET << std::endl;
                        break;
                    case DUPLICATE:
                        std::cout << "DUPLICATE" << RESET << std::endl;
                        break;
                    case OUT_OF_ORDER:
                        std::cout << "OUT_OF_ORDER" << RESET << std::endl;
                        break;
                    case NO_CLUE:
                        std::cout << "NO_CLUE" << RESET << std::endl;
                        break;
                }
    }

} // namespace multiplexer
