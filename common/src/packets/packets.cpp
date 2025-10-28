#include "packets.h"
#include "data_transfer/socket_utils.h"
#include <arpa/inet.h>
#include <netinet/in.h>
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

// ACK seq_num result new_balance sender_ip receiver_ip transfer_amount END
std::string ACK_Packet::to_string() {
    std::string packet_str;

    packet_str = "ACK";
    packet_str += " ";

    // sequence number
    packet_str += std::to_string(seq_num);
    packet_str += " ";

    // result
    packet_str += result;
    packet_str += " ";

    // new balance
    packet_str += std::to_string(new_balance);
    packet_str += " ";

    // sender_ip
    packet_str += addr_to_string(sender_ip);
    packet_str += " ";

    // receiver_ip
    packet_str += addr_to_string(receiver_ip);
    packet_str += " ";

    // transfer_amount
    packet_str += std::to_string(transfer_amount);
    packet_str += " ";

    packet_str += "END";
    return packet_str;
}

// KIL END
std::string KIL_Packet::to_string() {
    std::string packet_str;

    packet_str = "ACK";
    packet_str += " ";
    packet_str += "END";

    return packet_str;
}
std::string status_to_string(Packet_status s) {
    switch (s) {
    case VALID:
        return "VALID";
    case DUPLICATE:
        return "DUPLICATE";
    case OUT_OF_ORDER:
        return "OUT_OF_ORDER";
    case NO_CLUE:
    default:
        return "NO_CLUE";
    }
}
