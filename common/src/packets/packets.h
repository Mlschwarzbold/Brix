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
}; 

// REQ packet class
class REQ_Packet : public Packet {
  public:
    in_addr_t receiver_ip;
    long int transfer_amount;
    REQ_Packet(long int seq, in_addr_t ip, long int amount) {
        packet_type = REQ;
        seq_num = seq;
        receiver_ip = ip;
        transfer_amount = amount;
    }

    // to string method
    std::string to_string();
};  

// ANS packet class
class ANS_Packet : public Packet {
  public:
    bool result;
    long int new_balance;
    ANS_Packet(long int seq, bool res, long int balance) {
        packet_type = ANS;
        seq_num = seq;
        result = res;
        new_balance = balance;
    }
    // to string method
    std::string to_string();
};



#endif // PACKETS_H