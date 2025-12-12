#include "heartbeat.h"
#include "colors.h"
#include <iostream>
#include <pthread.h>
#include <arpa/inet.h>
#include <cstring>


namespace election {

    void heartbeat_receiver(int port){


        return;
    }

    void *start_heartbeat_receiver(void *arg) {
        int port = (*(int *)arg) + 2;

    #if _DEBUG
        std::cout << MAGENTA << "Starting Heartbeat Receiver on port " << port << RESET
                << std::endl;
    #endif
        (void)heartbeat_receiver(port);
        return 0;
}

} // namespace election