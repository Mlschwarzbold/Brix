#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include <iostream>
#include <pthread.h>
#include <arpa/inet.h>
#include <cstring>


namespace election {

    void heartbeat_receiver(int port);
    void *start_heartbeat_receiver(void *arg);

    bool heartbeat_test(const char *target_ip, int target_port, int timeout_ms, int tries);

} // namespace election

#endif