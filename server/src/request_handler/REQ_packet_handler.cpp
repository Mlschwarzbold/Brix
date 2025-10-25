#include "REQ_packet_handler.h"
#include "colors.h"
#include "db_manager/db_manager.h"
#include <iostream>

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

std::string process_db_transaction(in_addr_t sender_ip, REQ_Packet packet) {
    db_manager::db_record_response result;
    auto db = db_manager::DbManager::get_instance();

    result = db->make_transaction(sender_ip, packet.receiver_ip,
                                  packet.transfer_amount);

    switch (result.status_code) {

    case db_manager::db_record_response::SUCCESS:
        //
        return ACK_Packet(packet.seq_num, "SUCC", result.record->balance,
                          sender_ip, packet.receiver_ip, packet.transfer_amount)
            .to_string();
        break;
    case db_manager::db_record_response::INSUFFICIENT_BALANCE:
    case db_manager::db_record_response::UNKNOWN_RECEIVER:
        // Just reply with the client's balance and inform failure of
        return ACK_Packet(packet.seq_num, "FAIL", result.record->balance,
                          sender_ip, packet.receiver_ip, packet.transfer_amount)
            .to_string();
        // operation
        break;

    case db_manager::db_record_response::UNKNOWN_SENDER:
        std::cerr << RED
                  << "[REQUEST PROCESSOR]: Receveived request from ip not "
                     "registered in database! "
                  << RESET << std::endl;

        return ACK_Packet(packet.seq_num, "UKNW", 0, sender_ip,
                          packet.receiver_ip, packet.transfer_amount)
            .to_string();
        break;

    case db_manager::db_record_response::NOT_FOUND:
    case db_manager::db_record_response::DUPLICATE_IP:
    default:
        std::cerr << RED
                  << "[REQUEST PROCESSOR]: Invalid status code response "
                     "from database: "
                  << result.status_code << RESET << std::endl;
        return ACK_Packet(packet.seq_num, "UKNW", 0, sender_ip,
                          packet.receiver_ip, packet.transfer_amount)
            .to_string();
        break;
    }
}

void process_req_packet(const struct sockaddr_in sender_addr, REQ_Packet packet,
                        multiplexer::Packet_indexer *indexer,
                        int reply_sockfd) {

    in_addr_t sender_ip = sender_addr.sin_addr.s_addr;
    Packet_status status = indexer->index_packet(packet, sender_ip);

    std::string reply;

    switch (status) {
    case VALID:
        reply = process_db_transaction(sender_ip, packet);
        break;

    case DUPLICATE:
    case OUT_OF_ORDER:
        reply = ACK_Packet(packet.seq_num, "B_ID", 0, sender_ip,
                           packet.receiver_ip, packet.transfer_amount)
                    .to_string();
        break;

    case NO_CLUE:
        reply = ACK_Packet(packet.seq_num, "UKNW", 0, sender_ip,
                           packet.receiver_ip, packet.transfer_amount)
                    .to_string();
    }

    sendto(reply_sockfd, reply.data(), reply.length(), MSG_CONFIRM,
           (const struct sockaddr *)&sender_addr, sizeof(sender_addr));
}

} // namespace requests
