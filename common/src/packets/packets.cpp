#include "packets.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <string>


// translates a packet to string
// example:
// REQ seq_num sender_ip receiver_ip transfer_ammount END
// ANS seq_num status balance END
// ACK seq_num END
std::string packet_to_str(Packet packet){
    return "pass";
}