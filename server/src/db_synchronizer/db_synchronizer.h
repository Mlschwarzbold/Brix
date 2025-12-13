#ifndef DB_SYNCHRONIZER_H
#define DB_SYNCHRONIZER_H

#include "db_manager/db_manager.h"
#include <cstring>
namespace db_synchronizer {

class DB_Synchronizer {
  public:
    static DB_Synchronizer *setup(int port);
    static DB_Synchronizer *get_instance();

    ~DB_Synchronizer();

    static void broadcast_update(db_manager::db_snapshot snapshot);
    static void listen_for_updates();

  private:
    static DB_Synchronizer *instance;
    static bool isAlive;
    static int sockfd;
    static int port;
    static struct sockaddr_in servaddr;
    static struct sockaddr_in bcastaddr;
    DB_Synchronizer(int port);
};

void *start_server(void *arg);

} // namespace db_synchronizer

#endif // DB_MANAGER_H