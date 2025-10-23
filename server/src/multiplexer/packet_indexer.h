#ifndef PACKET_INDEXER_H
#define PACKET_INDEXER_H

#include <arpa/inet.h>
#include <packets/packets.h>
#include <unordered_map>

typedef struct _last_packet_info {
    long int last_seq_num;
    bool answer_sent;
    void *last_response_placeholder;
} Last_packet_info;

namespace multiplexer {

class Packet_indexer {
  public:
    // Hash table to store client IPs and their last valid request number
    std::unordered_map<in_addr_t, Last_packet_info> client_packet_table;

    Packet_indexer();

    Packet_status index_packet(REQ_Packet packet, in_addr_t client_ip);
    ;
};

} // namespace multiplexer

#endif // PACKET_INDEXER_H