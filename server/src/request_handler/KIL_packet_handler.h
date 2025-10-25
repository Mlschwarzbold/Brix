#ifndef KIL_PACKET_HANDLER_H
#define KIL_PACKET_HANDLER_H

#include "packets/packets.h"

namespace requests {

void process_kill_packet(const struct sockaddr_in sender_addr,
                         KIL_Packet packet, int reply_sockfd);

} // namespace requests

#endif // KIL_PACKET_HANDLER_H