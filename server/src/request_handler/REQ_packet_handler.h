#ifndef REQ_PACKET_HANDLER_H
#define REQ_PACKET_HANDLER_H

#include "packets/packets.h"
#include "packets/string_packets.h"
#include "multiplexer/packet_indexer.h"
#include "colors.h"
#include "db_manager/db_manager.h"
#include <arpa/inet.h>
#include <netinet/in.h>

namespace requests {

    void route_REQ_packet(REQ_Packet packet, in_addr_t client_ip, Last_packet_info *last_info);


} // namespace requests


#endif // REQ_PACKET_HANDLER_H