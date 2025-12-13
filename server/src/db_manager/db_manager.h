#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include "colors.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <string>
#include <unordered_map>

namespace db_manager {

const unsigned long int STARTING_BALANCE = 100000;
const unsigned int STARTING_REQUEST = 0;

// This is how a client is represented inside the database. They can be ordered
// or compared using the `client_ip` field.
struct client_record {
    in_addr_t ip;
    unsigned long int balance;
    unsigned int last_request;
};

// All responses from the database manager follow the same format:
// - A success field, indicating whether or not the operation was successful.
// - A record field, pointing towards a client record inside the database.
// - A description field, providing additional information regarding the result
//   of the request (mostly related to different types of error).
struct db_record_response {
    bool success;
    client_record record;
    enum Description {
        SUCCESS,
        DUPLICATE_IP,
        INSUFFICIENT_BALANCE,
        NOT_FOUND,
        UNKNOWN_SENDER,
        UNKNOWN_RECEIVER,
        BALANCE_CHECK
    } status_code;
};

struct db_metadata {
    int num_transactions;
    unsigned int total_transferred;
    int total_balance;
};

// Serializes to:
// `num_transactions total_transferred total_balance #`
// ` client1.ip client1.balance client1.last_request ;`
// ` client2.ip client2.balance client2.last_request ;`
// ...
// ` clientN.ip clientN.balance clientN.last_request ;`
struct db_snapshot {
    db_metadata metadata;
    std::unordered_map<in_addr_t, client_record> records;

    std::string to_string();
    static db_snapshot from_string(std::string serialized_db);
};

class DbManager {
  public:
    ~DbManager();
    static DbManager *get_instance();

    // Registers a new entry for the client in the database records.
    // - If the client already exists, returns false in success and the
    // existing client on the server in result.
    // - If the client does not exist, returns true in success and a copy of
    // the newly inserted client in result.
    // - Possible failures:
    //   # `DUPLICATE_IP` - The provided ip is already registered in the
    //   database.
    const db_record_response register_client(in_addr_t client_ip);

    // Searches for a given client inside the database.
    // - If found, returns true in success and an iterator to the record inside
    // result.
    // - If NOT found, returns false in success and the contents of result are
    // invalid.
    // - Possible failures:
    //   # `NOT_FOUND` - The provided ip was not registered inside the database.
    const db_record_response get_client_info(in_addr_t client_ip);

    // Registers a transaction between two clients.
    // Transfers `amount` from the sender's balance to the receiver's balance,
    // first validating it is enough to actually fulfill the request. If the
    // request is successful (i.e. the sender had enough balance to send to that
    // ip AND the sender and receiver are registered in the system), returns
    // true inside success, otherwise, returns false. Either way, it returns a
    // copy of the sender in the result field, with the updated balance in case
    // of success.
    // - Possible failures:
    //   # `INSUFFICIENT_BALANCE` - The sender did not have sufficient balance
    //     to fulfill the transaction
    //   # `UNKNOWN_SENDER` - The ip provided for the sender is not registered
    //     in the database.
    //   # `UNKNOWN_RECEIVER` - The ip provided for the receiver is not
    //   registered in the database.
    //   # `DUPLICATE_IP` - Sender and receiver IPs are the
    //   # `BALANCE_CHECK` - Not a transfer, just a balance check (transfer
    //   amount is 0)
    const db_record_response make_transaction(in_addr_t sender_ip,
                                              in_addr_t receiver_ip,
                                              unsigned long transfer_amount);

    // Removes a client's entry from the database records. The value of `record`
    // will always be null, independent of success or failure.
    // - Possible failures:
    //   # `NOT_FOUND` - The client was not found inside the database records.
    //     Either it was already removed or it was never registered.
    const db_record_response remove_client(in_addr_t client_ip);

    // Retrieves data about the database:
    // - Number of transactions
    // - Total amount of money transferred
    // - Total balance in the database
    const db_metadata get_db_metadata();

    const db_snapshot get_db_snapshot();

    void load_snapshot(db_snapshot snapshot);

  private:
    DbManager();

    std::unordered_map<in_addr_t, pthread_mutex_t *> client_locks;
    pthread_mutex_t database_access_lock;
    pthread_mutex_t database_metadata_lock;

    // The records are kept in an unordered map (indexed by their client_ip,
    // using a hash table  for efficient storage
    std::unordered_map<in_addr_t, client_record> database_records;

    unsigned int total_transferred;
    int num_transactions;
    int total_balance;

    void print_record(client_record client);

    // Helper functions to give more semantic to database locking operations
    void lock_database();
    void unlock_database();

    void lock_metadata();
    void unlock_metadata();

    void lock_client(in_addr_t client_ip);
    void unlock_client(in_addr_t client_ip);
};

} // namespace db_manager

#endif // DB_MANAGER_H