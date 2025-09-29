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
struct db_response {
    bool success;
    std::set<client_record>::iterator result;
};

// Registers a new entry for the client in the database records.
// - If the client already exists, returns false in success and the existing
// client on the server in result.
// - If the client does not exist, returns true in success and a copy of the
// newly inserted client in result.
const db_response register_client(in_addr_t client_ip);

// Searches for a given client inside the database.
// - If found, returns true in success and an iterator to the record inside
// result.
// - If NOT found, returns false in success and the contents of result are
// invalid.
const db_response get_client_info(in_addr_t client_ip);
} // namespace db_manager

#endif // DB_MANAGER_H