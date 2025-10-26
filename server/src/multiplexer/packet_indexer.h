#ifndef PACKET_INDEXER_H
#define PACKET_INDEXER_H

#include <unordered_map>
#include <arpa/inet.h>
#include <packets/packets.h>


typedef struct _last_packet_info {
    long int last_seq_num;
    bool answer_sent;
    void* last_response_placeholder; // refactor into char with max size 
} Last_packet_info;

typedef struct _indexing_result {
    Packet_status status;
    Last_packet_info* last_info;
} Indexing_result;

namespace multiplexer {

    class Packet_indexer {
    public:
    
        // Hash table to store client IPs and their last valid request number
        std::unordered_map<in_addr_t, Last_packet_info> client_packet_table;

        Packet_indexer();

        Indexing_result index_packet(REQ_Packet packet, in_addr_t client_ip);


    };

    void print_status(Indexing_result result);

} // namespace multiplexer

#endif // PACKET_INDEXER_H