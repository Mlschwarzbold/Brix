#include "packets.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <string>


// translates a packet to string
// example:
// REQ seq_num sender_ip receiver_ip transfer_amount END
// ANS seq_num result new_balance END
// ACK seq_num END
std::string packet_to_str(Packet packet){
    std::string packet_str;
    char ip_str[INET_ADDRSTRLEN];
    switch (packet.packet_type) {
    case REQ:
        packet_str = "REQ";
        packet_str += " ";
        // sequence number
        packet_str += std::to_string(packet.seq_num);
        packet_str += " ";
        // convert receiver ip to str and append
        inet_ntop(AF_INET, &packet.receiver_ip, ip_str, INET_ADDRSTRLEN);
        packet_str += ip_str;
        packet_str += " ";
        // transfer amount
        packet_str += std::to_string(packet.transfer_amount);
        packet_str += " END";
        break;
    case ANS:
        packet_str = "ANS";

        break;
    case ACK:
        packet_str = "ACK";


        break;

    default:
        packet_str = "ERROR";
        break;
    }

    return packet_str;
}