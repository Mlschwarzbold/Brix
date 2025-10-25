#ifndef KIL_PACKET_HANDLER_H
#define KIL_PACKET_HANDLER_H

#include "packets/packets.h"

namespace requests {
typedef struct {
    const struct sockaddr_in sender_addr;
    KIL_Packet packet;
    int reply_sockfd;
} process_kill_packet_params;

void *process_kill_packet(void *arg);

} // namespace requests

#endif // KIL_PACKET_HANDLER_H