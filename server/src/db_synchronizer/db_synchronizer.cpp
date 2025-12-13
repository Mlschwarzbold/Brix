
#include "db_synchronizer.h"
#include "data_transfer/socket_utils.h"
#include "iostream"

namespace db_synchronizer {
const int MAXLINE = 1024;
const int BROADCAST_TIMEOUT = 5000;

DB_Synchronizer *DB_Synchronizer::instance;
int DB_Synchronizer::sockfd;
int DB_Synchronizer::port;
struct sockaddr_in DB_Synchronizer::servaddr;
struct sockaddr_in DB_Synchronizer::bcastaddr;
bool DB_Synchronizer::isAlive;

void *start_server(void *arg) {
    DB_Synchronizer::listen_for_updates();

    return nullptr;
}

DB_Synchronizer::DB_Synchronizer(int port) {
    this->port = port + 3;
    isAlive = true;

    // Creating socket file descriptor
    sockfd = create_udp_socket();

    // Enable broadcast
    enable_broadcast(sockfd);

    set_timeout(sockfd, BROADCAST_TIMEOUT);

    memset(&servaddr, 0, sizeof(servaddr));

    this->servaddr = create_sockaddr("255.255.255.255", this->port);
    this->servaddr.sin_addr.s_addr = INADDR_ANY;

    this->bcastaddr = create_sockaddr("255.255.255.255", this->port);

    bind_to_sockaddr(sockfd, &this->servaddr);
}

void DB_Synchronizer::broadcast_update(db_manager::db_snapshot snapshot) {
    std::string serialized_snapshot = snapshot.to_string();

    std::cout << GREEN << "[DB SYNCHRONIZER]: Broadcasting update." << RESET
              << std::endl;

    sendto(sockfd, serialized_snapshot.data(), serialized_snapshot.length(), 0,
           (const struct sockaddr *)&bcastaddr, sizeof(bcastaddr));

    std::cout << GREEN << "[DB SYNCHRONIZER]: Finished broadcast." << RESET
              << std::endl;
}

void DB_Synchronizer::listen_for_updates() {
    int n;
    char buffer[MAXLINE + 1];
    db_manager::DbManager *db = db_manager::DbManager::get_instance();
    struct sockaddr_in src;
    socklen_t srclen = sizeof(src);

    while (isAlive) {
        n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL,
                     (struct sockaddr *)&src, &srclen);
        if (n < 0) {
            std::cerr << YELLOW << "Timeout or error fetching updates... \n"
                      << RESET << std::endl;
        } else {
            // Ignore messages from ourselves
            if (addr_to_string(src.sin_addr.s_addr) != get_self_ip()) {
                std::cout << GREEN
                          << "[DB SYNCHRONIZER] Message received: " << buffer
                          << RESET << std::endl;

                db->load_snapshot(db_manager::db_snapshot::from_string(buffer));

                auto snapshot = db->get_db_snapshot();
                std::cout << snapshot.to_string() << std::endl;
            }
        }
    }
}

DB_Synchronizer::~DB_Synchronizer() { isAlive = false; }

DB_Synchronizer *DB_Synchronizer::setup(int port) {
    static DB_Synchronizer *instance = new DB_Synchronizer(port);
    return instance;
}

DB_Synchronizer *DB_Synchronizer::get_instance() {
    if (instance != nullptr) {
        return instance;
    }

    return setup(9999);
};
} // namespace db_synchronizer
