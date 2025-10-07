#ifndef PACKETS_H
#define PACKETS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <string>

// Packet type enum
typedef enum _packet_type {
        REQ,
        ANS,
        ACK
    } Packet_type;


// Packet class
class Packet {        
  public:        
    Packet_type packet_type;  
    long int seq_num;
    in_addr_t receiver_ip;
    unsigned long int transfer_amount;   
    Packet(Packet_type type, long int seq, in_addr_t r_ip, unsigned long int amount) {
      seq_num = seq;
      packet_type = type;
      receiver_ip = r_ip;
      transfer_amount = amount;
    }
}; 

// Packet to string
std::string packet_to_str(Packet packet);

// String to packet

#endif // PACKETS_H