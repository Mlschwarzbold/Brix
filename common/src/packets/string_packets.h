#ifndef STRING_PACKETS_H
#define STRING_PACKETS_H

#include "packets.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

class String_Packet : public std::string {
  public:
    String_Packet(std::string str) : std::string(str) {}

    // to packet method
    REQ_Packet to_REQ_Packet();
    ACK_Packet to_ACK_Packet();
    KIL_Packet to_KIL_Packet();

    Packet_type type();
};

#endif // STRING_PACKETS_H