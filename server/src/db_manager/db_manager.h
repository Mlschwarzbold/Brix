#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <set>
namespace db_manager {
void demo();

const unsigned long int STARTING_BALANCE = 200;
const unsigned int STARTING_REQUEST = 1;

// This is how a client is represented inside the database. They can be ordered
// or compared using the `client_ip` field.
struct client_record {
    in_addr_t client_ip;
    unsigned long int client_balance;
    unsigned int last_request;
};

// All responses from the database manager follow the same format:
// - A success field, indicating whether or not the operation was successful.
// - A result field, poiting towards a client record inside the database.
// - A description field, providing additional information regarding the result
//   of the request (mostly related to different types of error).
struct db_response {

    bool success;
    std::set<client_record>::iterator result;
    enum Description {
        SUCCESS,
        DUPLICATE_IP,
        INSUFFICIENT_BALANCE,
        NOT_FOUND,
        UNKNOWN_SENDER,
        UNKNOWN_RECEIVER,
    } description;
};

// Registers a new entry for the client in the database records.
// - If the client already exists, returns false in success and the existing
// client on the server in result.
// - If the client does not exist, returns true in success and a copy of the
// newly inserted client in result.
// - Possible failures:
//   # `DUPLICATE_IP` - The provided ip is already registered in the database.
const db_response register_client(in_addr_t client_ip);

// Searches for a given client inside the database.
// - If found, returns true in success and an iterator to the record inside
// result.
// - If NOT found, returns false in success and the contents of result are
// invalid.
// - Possible failures:
//   # `NOT_FOUND` - The provided ip was not registered inside the database.
const db_response get_client_info(in_addr_t client_ip);

// Registers a transaction between two clients.
// Transfers `amount` from the sender's balance to the receiver's balance, first
// validating it is enought to actually fulfill the request.
// FIXME: I am still not 100% sure this is the correct way to implement this.
// If the request is sucessful (i.e. the sender had enough balance to send to
// that ip AND the sender and receiver are registered in the system), returns
// true inside success, otherwise, returns false. Either way, it returns a copy
// of the sender in the result field, with the updated balance in case of
// success.
// - Possible failures:
//   # `INSUFFICIENT_BALANCE` - The sender did not have sufficient balance to
//     fulfill the transaction
//   # `UNKNOWN_SENDER` - The ip provided for the sender is not registered in
//     the database.
//   # `UNKNOWN_RECEIVER` - The ip provided for the receiver is not registered
//     in the database.
const db_response make_transaction(in_addr_t sender_ip, in_addr_t receiver,
                                   long int amount);
} // namespace db_manager

#endif // DB_MANAGER_H