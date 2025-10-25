#include "string_packets.h"
#include "packets.h"
#include <sstream>
#include <stdexcept>
#include <vector>

// translates a string to a REQ packet
// example:
// REQ seq_num sender_ip receiver_ip transfer_amount END
REQ_Packet String_Packet::to_REQ_Packet() {
    // Split the string into tokens
    std::istringstream iss(*this);
    std::string token;
    std::vector<std::string> tokens;

    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.size() != 5 || tokens[0] != "REQ" || tokens[4] != "END") {
        throw std::invalid_argument("Invalid REQ packet format");
        // it is surprisingly fast for the c++ compiler to handle exceptions, it
        // cost literally nothing (on normal case) according to stackoverflow
        // (copilot REALLY wants to hallucinate a stackoverflow link here)
    }

    long int seq_num = std::stol(tokens[1]);
    in_addr_t receiver_ip = inet_addr(tokens[2].c_str()); // Get char*`d
    unsigned long int transfer_amount = std::stoul(tokens[3]);

    return REQ_Packet(seq_num, receiver_ip, transfer_amount);
}

// translates a string to an ACK packet
// example:
// ACK seq_num result new_balance sender_ip receiver_ip transfer_amount END
ACK_Packet String_Packet::to_ACK_Packet() {
    // Split the string into tokens
    std::istringstream iss(*this);
    std::string token;
    std::vector<std::string> tokens;

    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.size() != 8 || tokens[0] != "ACK" || tokens[7] != "END") {
        throw std::invalid_argument("Invalid ACK packet format");
    }

    long int seq_num = std::stol(tokens[1]);
    std::string result = tokens[2];
    unsigned long int new_balance = std::stoul(tokens[3]);
    in_addr_t sender_ip = inet_addr(tokens[4].c_str());
    in_addr_t receiver_ip = inet_addr(tokens[5].c_str());
    int transfer_amount = std::stoi(tokens[6]);

    return ACK_Packet(seq_num, result, new_balance, sender_ip, receiver_ip,
                      transfer_amount);
};

KIL_Packet String_Packet::to_KIL_Packet() {
    // Split the string into tokens
    std::istringstream iss(*this);
    std::string token;
    std::vector<std::string> tokens;

    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.size() != 2 || tokens[0] != "KIL" || tokens[1] != "END") {
        throw std::invalid_argument("Invalid KIL packet format");
    }

    return KIL_Packet();
};

Packet_type String_Packet::type() {
    // Determine the packet type based on the first 3 characters
    if (this->substr(0, 3) == "REQ") {
        return REQ;
    } else if (this->substr(0, 3) == "ACK") {
        return ACK;
    } else if (this->substr(0, 3) == "ERR") {
        return ERR;
    } else if (this->substr(0, 3) == "KIL") {
        return KIL;
    } else {
        return ERR;
    }
}
