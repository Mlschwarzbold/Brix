#include "string_packets.h"

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
            // it is surprisingly fast for the c++ compiler to handle exceptions, it cost literally nothing (on normal case)
            // according to stackoverflow
            // (copilot REALLY wants to hallucinate a stackoverflow link here)
        }

        long int seq_num = std::stol(tokens[1]);
        in_addr_t receiver_ip = inet_addr(tokens[2].c_str()); // Get char*`d
        unsigned long int transfer_amount = std::stoul(tokens[3]);

        return REQ_Packet(seq_num, receiver_ip, transfer_amount);
    }