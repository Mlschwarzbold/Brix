#include "REQ_packet_handler.h"
#include "colors.h"
#include "db_manager/db_manager.h"
#include <iostream>
#include <sstream>

namespace requests {

void print_status(Packet_status status) {
    std::cout << GREEN << "Packet status: ";
    switch (status) {
    case VALID:
        std::cout << "VALID" << RESET << std::endl;
        break;
    case DUPLICATE:
        std::cout << "DUPLICATE" << RESET << std::endl;
        break;
    case OUT_OF_ORDER:
        std::cout << "OUT_OF_ORDER" << RESET << std::endl;
        break;
    case NO_CLUE:
        std::cout << "NO_CLUE" << RESET << std::endl;
        break;
    }
}

std::string build_reply(bool success, int last_processed_transaction,
                        int new_client_balance) {
    std::ostringstream reply_constructor;

    std::string result;
    if (success) {
        result = "SUCC";
    } else {
        result = "FAIL";
    }

    reply_constructor << "ACK " << result << " " << last_processed_transaction
                      << " " << new_client_balance << " END\n";
    return reply_constructor.str();
}

void process_req_packet(struct sockaddr_in *sender_addr, REQ_Packet packet,
                        multiplexer::Packet_indexer *indexer,
                        int reply_sockfd) {

    auto sender_ip = sender_addr->sin_addr.s_addr;
    auto status = indexer->index_packet(packet, sender_ip);

    print_status(status);
    auto db = db_manager::DbManager::get_instace();

    std::string reply = "Get boom'd idiot!";
    db_manager::db_record_response result;

    switch (status) {
    case VALID:
        result = db->make_transaction(sender_ip, packet.receiver_ip,
                                      packet.transfer_amount);
        if (result.success) {
            reply = build_reply(true, packet.seq_num, result.record->balance);
        }
        break;

    case DUPLICATE:
    case OUT_OF_ORDER:
    case NO_CLUE:
        return;
    }

    sendto(reply_sockfd, reply.data(), reply.length(), MSG_CONFIRM,
           (const struct sockaddr *)sender_addr, sizeof(*sender_addr));
}

} // namespace requests
