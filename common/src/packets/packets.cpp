#include "packets.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <string>


// translates a packet to string
// example:
// REQ seq_num sender_ip receiver_ip transfer_amount END
std::string REQ_Packet::to_string() {
    std::string packet_str;
    char ip_str[INET_ADDRSTRLEN];

    packet_str = "REQ";
    packet_str += " ";
    // sequence number
    packet_str += std::to_string(seq_num);
    packet_str += " ";
    // convert receiver ip to str and append
    inet_ntop(AF_INET, &receiver_ip, ip_str, INET_ADDRSTRLEN);
    packet_str += ip_str;
    packet_str += " ";
    // transfer amount
    packet_str += std::to_string(transfer_amount);
    packet_str += " END";

    return packet_str;

}


// ANS seq_num result new_balance END
std::string ANS_Packet::to_string() {
    std::string packet_str;

    packet_str = "ANS";
    packet_str += " ";
    // sequence number
    packet_str += std::to_string(seq_num);
    packet_str += " ";
    // result
    packet_str += (result ? "SUCC" : "FAIL");
    packet_str += " ";
    // new balance
    packet_str += std::to_string(new_balance);
    packet_str += " END";
    return packet_str;
}
// ACK seq_num END