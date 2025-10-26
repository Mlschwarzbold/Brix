#include "REQ_packet_handler.h"
#include "colors.h"
#include "data_transfer/socket_utils.h"
#include "date_time_utils.h"
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

ACK_Packet process_db_transaction(in_addr_t sender_ip, REQ_Packet packet,
                                  db_manager::DbManager *db) {
    db_manager::db_record_response result;

    result = db->make_transaction(sender_ip, packet.receiver_ip,
                                  packet.transfer_amount);

    switch (result.status_code) {

    case db_manager::db_record_response::SUCCESS:
        return ACK_Packet(result.record.last_request, "SUCCESS",
                          result.record.balance, sender_ip, packet.receiver_ip,
                          packet.transfer_amount);

    case db_manager::db_record_response::INSUFFICIENT_BALANCE:
        return ACK_Packet(result.record.last_request, "INSUFFICIENT_BALANCE",
                          result.record.balance, sender_ip, packet.receiver_ip,
                          packet.transfer_amount);

    case db_manager::db_record_response::UNKNOWN_RECEIVER:
        // Just reply with the client's balance and inform failure of
        // operation
        return ACK_Packet(result.record.last_request, "RECEIVER_NOT_REGISTERED",
                          result.record.balance, sender_ip, packet.receiver_ip,
                          packet.transfer_amount);

    case db_manager::db_record_response::DUPLICATE_IP:
        return ACK_Packet(result.record.last_request, "BAD_REQUEST",
                          result.record.balance, sender_ip, packet.receiver_ip,
                          packet.transfer_amount);

    case db_manager::db_record_response::UNKNOWN_SENDER:
        std::cerr << RED
                  << "[REQUEST PROCESSOR]: Receveived request from ip not "
                     "registered in database! "
                  << RESET << std::endl;

        return ACK_Packet(0, "SENDER_NOT_REGISTERED", 0, sender_ip,
                          packet.receiver_ip, packet.transfer_amount);

    case db_manager::db_record_response::NOT_FOUND:
    default:
        std::cerr << RED
                  << "[REQUEST PROCESSOR]: Invalid status code response "
                     "from database: "
                  << result.status_code << RESET << std::endl;
        return ACK_Packet(0, "UKNW", 0, sender_ip, packet.receiver_ip,
                          packet.transfer_amount);
    }
}

void print_request_info(in_addr_t sender_ip, REQ_Packet packet, bool is_dup,
                        bool is_out_of_order, db_manager::DbManager *db) {
    // 2025-09-11 18:37:01 client 10.1.1.2 id_req 1 dest 10.1.1.3 value 10
    // num transactions 1 total transferred 10 total balance 300
    // Request info
    std::cout << CYAN;
    std::cout << getCurrentDateString() << " " << getCurrentTimeString();
    std::cout << " client " << addr_to_string(sender_ip);
    std::cout << " id_req " << packet.seq_num;
    if (is_dup) {
        std::cout << RED << " DUPLICATE! " << CYAN;
    } else if (is_out_of_order) {
        std::cout << RED << " OUT_OF_ORDER! " << CYAN;
    }
    std::cout << " dest " << addr_to_string(packet.receiver_ip);
    std::cout << " value " << packet.transfer_amount;
    std::cout << RESET << std::endl;

    // Server info
    db_manager::db_metadata db_metadata = db->get_db_metadata();
    std::cout << BLUE;
    std::cout << "num_transactions " << db_metadata.num_transactions;
    std::cout << " total_transferred " << db_metadata.total_transferred;
    std::cout << " total_balance " << db_metadata.total_balance;
    std::cout << RESET << std::endl;
}

Packet_status check_packet_status(REQ_Packet packet,
                                  db_manager::client_record client) {
    int received_seq_num = packet.seq_num;
    int expected_seq_num = client.last_request + 1;

    if (received_seq_num == expected_seq_num) {
        return VALID;
    } else if (received_seq_num < expected_seq_num) {
        return DUPLICATE;
    } else {
        return OUT_OF_ORDER;
    }
}

void *process_req_packet(void *arg) {

    process_req_packet_params params = *(process_req_packet_params *)arg;

    struct sockaddr_in sender_addr = params.sender_addr;
    REQ_Packet packet = params.packet;
    int reply_sockfd = params.reply_sockfd;
    db_manager::DbManager *db = db_manager::DbManager::get_instance();

    auto client_record =
        db->get_client_info(sender_addr.sin_addr.s_addr).record;
    ;

    in_addr_t sender_ip = sender_addr.sin_addr.s_addr;
    Packet_status status = check_packet_status(packet, client_record);

    ACK_Packet reply;
    // std::cout << "Receiver: " << addr_to_string(packet.receiver_ip)
    //           << std::endl;

    switch (status) {
    case VALID:
        reply = process_db_transaction(sender_ip, packet, db);
        print_request_info(sender_ip, packet, false, false, db);
        break;

    case DUPLICATE:
        reply = ACK_Packet(client_record.last_request, "DUPLICATE",
                           client_record.balance, sender_ip, packet.receiver_ip,
                           packet.transfer_amount);
        print_request_info(sender_ip, packet, true, false, db);
        break;

    case OUT_OF_ORDER:
        reply = ACK_Packet(client_record.last_request, "OUT_OF_ORDER",
                           client_record.balance, sender_ip, packet.receiver_ip,
                           packet.transfer_amount);
        print_request_info(sender_ip, packet, false, true, db);

        break;

    case NO_CLUE:
        reply = ACK_Packet(client_record.last_request, "UNKOWN_STATUS",
                           client_record.balance, sender_ip, packet.receiver_ip,
                           packet.transfer_amount);
        std::cerr << RED << "[REQUEST PROCESSOR] INVALID STATUS!" << std::endl;
    }

    std::string reply_string = reply.to_string();

    sendto(reply_sockfd, reply_string.data(), reply_string.length(),
           MSG_CONFIRM, (const struct sockaddr *)&sender_addr,
           sizeof(sender_addr));

    free(arg);
    return nullptr;
}

} // namespace requests
