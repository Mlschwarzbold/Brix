#ifndef REQ_PACKET_HANDLER_H
#define REQ_PACKET_HANDLER_H

#include "multiplexer/packet_indexer.h"
#include "packets/packets.h"
namespace requests {

void print_status(Packet_status status);
void process_req_packet(const struct sockaddr_in sender_addr, REQ_Packet packet,
                        multiplexer::Packet_indexer *indexer, int reply_sockfd);

} // namespace requests

#endif // REQ_PACKET_HANDLER_H