#include "KIL_packet_handler.h"
#include "colors.h"
#include "db_manager/db_manager.h"

namespace requests {

void process_kill_packet(const struct sockaddr_in sender_addr,
                         KIL_Packet packet, int reply_sockfd) {

    in_addr_t sender_ip = sender_addr.sin_addr.s_addr;

    auto db = db_manager::DbManager::get_instance();
    auto result = db->remove_client(sender_ip);

    std::string reply;

    if (result.success) {
        reply = ACK_Packet(packet.seq_num, "KILL", result.record.balance,
                           sender_ip, 0, -result.record.balance)
                    .to_string();
    } else {
        reply =
            ACK_Packet(packet.seq_num, "KILL", 0, sender_ip, 0, 0).to_string();
    }

    sendto(reply_sockfd, reply.data(), reply.length(), MSG_CONFIRM,
           (const struct sockaddr *)&sender_addr, sizeof(sender_addr));
}

} // namespace requests
