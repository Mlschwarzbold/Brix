#include <arpa/inet.h>
#include <unordered_map>
#include "packet_indexer.h"

namespace multiplexer {

    PacketIndexer::PacketIndexer() {
        client_packet_table = std::unordered_map<in_addr_t, int>();
    }

    Packet_status PacketIndexer::index_packet(REQ_Packet packet, in_addr_t client_ip){
        
        // Check if the client IP is already in the table
        if (client_packet_table.find(client_ip) != client_packet_table.end()) {
            // If client has already sent packets, check sequence number
            if (packet.seq_num == client_packet_table[client_ip] + 1) {
                // If seq num is correct, update the table and return 1 (valid packet)
                client_packet_table[client_ip] = packet.seq_num;
                return VALID; 
            } else {
                if (packet.seq_num < client_packet_table[client_ip]) {
                    return DUPLICATE; 
                } else {
                    return OUT_OF_ORDER;
                }
            }
        } else {
            // New client, check if seq num is 1
            if (packet.seq_num == 1) {
                client_packet_table[client_ip] = packet.seq_num;
                return VALID;
            } else {
                return OUT_OF_ORDER;
            }
        }


       return NO_CLUE;
    }

} // namespace multiplexer
