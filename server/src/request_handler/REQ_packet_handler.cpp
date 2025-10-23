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

std::string build_reply(std::string result, int last_processed_transaction,
                        int new_client_balance) {
    std::ostringstream reply_constructor;

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
            reply = build_reply("SUCC", packet.seq_num, result.record->balance);
        } else {
            reply = build_reply("FAIL", packet.seq_num, result.record->balance);
        }
        break;

    case DUPLICATE:
    case OUT_OF_ORDER:
        reply = build_reply(
            "B_ID", indexer->client_packet_table.at(sender_ip).last_seq_num, 0);
        break;
    case NO_CLUE:
        return;
    }
    sendto(reply_sockfd, reply.data(), reply.length(), MSG_CONFIRM,
           (const struct sockaddr *)sender_addr, sizeof(*sender_addr));
}

} // namespace requests
