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

void print_req(struct sockaddr_in *);

void process_req_packet(const struct sockaddr_in sender_addr, REQ_Packet packet,
                        multiplexer::Packet_indexer *indexer,
                        int reply_sockfd) {

    auto sender_ip = sender_addr.sin_addr.s_addr;
    auto status = indexer->index_packet(packet, sender_ip);

    auto db = db_manager::DbManager::get_instance();

    std::string reply = "Get boom'd idiot!";
    db_manager::db_record_response result;

    switch (status) {
    case VALID:
        result = db->make_transaction(sender_ip, packet.receiver_ip,
                                      packet.transfer_amount);
        switch (result.status_code) {

        case db_manager::db_record_response::SUCCESS:
            //
            reply = build_reply("SUCC", packet.seq_num, result.record->balance);
            break;
        case db_manager::db_record_response::INSUFFICIENT_BALANCE:
        case db_manager::db_record_response::UNKNOWN_RECEIVER:
            // Just reply with the client's balance and inform failure of
            reply = build_reply("FAIL", packet.seq_num, result.record->balance);
            // operation
            break;

        case db_manager::db_record_response::UNKNOWN_SENDER:
            std::cerr << RED
                      << "[REQUEST PROCESSOR]: Receveived request from ip not "
                         "registered in database: "
                      << inet_ntoa(sender_addr.sin_addr) << RESET << std::endl;
            return;
        case db_manager::db_record_response::NOT_FOUND:
        case db_manager::db_record_response::DUPLICATE_IP:
            std::cerr << RED
                      << "[REQUEST PROCESSOR]: Invalid status code response "
                         "from database: "
                      << result.status_code << RESET << std::endl;
            return;
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
           (const struct sockaddr *)&sender_addr, sizeof(sender_addr));
}

} // namespace requests
