#ifndef STRING_PACKETS_H
#define STRING_PACKETS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <stdexcept>
#include <sstream>
#include <vector>
#include "packets.h"

class String_Packet : public std::string {
  public:
    String_Packet(std::string str) : std::string(str) {}

    // to packet method
    REQ_Packet to_REQ_Packet();
    
};

#endif // STRING_PACKETS_H