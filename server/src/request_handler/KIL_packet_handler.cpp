#include "KIL_packet_handler.h"
#include "colors.h"
#include "data_transfer/socket_utils.h"
#include "db_manager/db_manager.h"
#include "db_synchronizer/db_synchronizer.h"
#include "iostream"

namespace requests {

void *process_kill_packet(void *arg) {

    process_kill_packet_params params = *(process_kill_packet_params *)arg;

    struct sockaddr_in sender_addr = params.sender_addr;
    KIL_Packet packet = params.packet;
    int reply_sockfd = params.reply_sockfd;

    in_addr_t sender_ip = sender_addr.sin_addr.s_addr;

#if _DEBUG
    std::cout << MAGENTA << "[KILL PROCESSOR]: Removing client "
              << addr_to_string(sender_ip) << " from database..." << RESET
              << std::endl;
#endif

    auto db = db_manager::DbManager::get_instance();
    auto result = db->remove_client(sender_ip);

    std::string reply;

    switch (result.status_code) {

    case db_manager::db_record_response::SUCCESS:
        reply = ACK_Packet(packet.seq_num, "KILL", result.record.balance,
                           sender_ip, 0, result.record.balance)
                    .to_string();

#if _DEBUG
        std::cout << MAGENTA << "[KILL PROCESSOR]: Succesfully removed client "
                  << addr_to_string(sender_ip) << " from database." << RESET
                  << std::endl;
#endif
        break;

    case db_manager::db_record_response::NOT_FOUND:
        reply = ACK_Packet(packet.seq_num, "SENDER_NOT_REGISTERED", 0,
                           sender_ip, 0, 0)
                    .to_string();
#if _DEBUG
        std::cout << MAGENTA
                  << "[KILL PROCESSOR]: Client not found on database: "
                  << addr_to_string(sender_ip) << "!" << RESET << std::endl;
#endif
        break;

    case db_manager::db_record_response::DUPLICATE_IP:
    case db_manager::db_record_response::INSUFFICIENT_BALANCE:
    case db_manager::db_record_response::UNKNOWN_SENDER:
    case db_manager::db_record_response::UNKNOWN_RECEIVER:
    case db_manager::db_record_response::BALANCE_CHECK:
        reply = ACK_Packet(packet.seq_num, "SERVER_ERROR", 0, sender_ip, 0, 0)
                    .to_string();
#if _DEBUG
        std::cout << MAGENTA
                  << "[KILL PROCESSOR]: Got unexpected result from database!"
                  << std::endl;
#endif
    }

    sendto(reply_sockfd, reply.data(), reply.length(), MSG_CONFIRM,
           (const struct sockaddr *)&sender_addr, sizeof(sender_addr));

    if (result.status_code == db_manager::db_record_response::SUCCESS) {
        auto db_sync = db_synchronizer::DB_Synchronizer::get_instance();
        auto db_snapshot = db->get_db_snapshot();
        db_sync->broadcast_update(db_snapshot);
    }

    return nullptr;
}

} // namespace requests
