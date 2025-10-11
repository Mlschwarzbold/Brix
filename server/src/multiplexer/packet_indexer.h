#ifndef PACKET_INDEXER_H
#define PACKET_INDEXER_H

#include <unordered_map>
#include <arpa/inet.h>
#include <packets/packets.h>


namespace multiplexer {

    class PacketIndexer {
    public:
    
        // Hash table to store client IPs and their last valid request number
        std::unordered_map<in_addr_t, int> client_packet_table;

        PacketIndexer();

        Packet_status index_packet(REQ_Packet packet, in_addr_t client_ip);

    };

} // namespace multiplexer

#endif // PACKET_INDEXER_H