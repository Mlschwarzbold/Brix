#ifndef HEARTBEAT_H
#define HEARTBEAT_H

namespace election {

    void heartbeat_receiver(int port);
    void *start_heartbeat_receiver(void *arg);

} // namespace election

#endif