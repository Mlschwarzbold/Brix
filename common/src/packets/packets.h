#ifndef PACKETS_H
#define PACKETS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

// Packet type enum
typedef enum _packet_type { REQ, ACK, ERR, KIL } Packet_type;

// Packet status enum
typedef enum _packet_status {
    VALID,
    DUPLICATE,
    OUT_OF_ORDER,
    NO_CLUE
} Packet_status;

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

// ACK packet class
class ACK_Packet : public Packet {
  public:
    std::string result;
    unsigned long int new_balance;
    in_addr_t sender_ip;
    in_addr_t receiver_ip;
    int transfer_amount;

    ACK_Packet(long int seq, std::string res, unsigned long int balance,
               in_addr_t sender_ip, in_addr_t receiver_ip,
               int transfer_amount) {
        packet_type = ACK;
        seq_num = seq;
        result = res;
        new_balance = balance;
        this->sender_ip = sender_ip;
        this->receiver_ip = sender_ip;
        this->transfer_amount = transfer_amount;
    }

    // to string method
    std::string to_string();
};

class KIL_Packet : public Packet {
  public:
    KIL_Packet() {
        packet_type = KIL;
        seq_num = -1;
    };

    // to string method
    std::string to_string();
};

#endif // PACKETS_H