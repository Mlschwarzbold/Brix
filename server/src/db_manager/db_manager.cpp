

#include "db_manager.h"

namespace db_manager {

DbManager::DbManager() {
    // The records are kept in an unordere map (indexed by their client_ip,
    // using a hash table  for efficient storage
    database_records = std::unordered_map<in_addr_t, client_record>();

    /// Initialize global acessor, used to indicate when a thead is in the
    /// process of acquiring locks
    pthread_mutex_init(&database_access_lock, NULL);

    // Client locks are used to avoid two threads trying to read or modify info
    // about two clients at the same time.
    client_locks = std::unordered_map<in_addr_t, pthread_mutex_t *>();

    // Register a few clients
    register_client(inet_addr("10.0.0.1"));
    register_client(inet_addr("10.0.0.2"));

    // Make a transaction
    make_transaction(inet_addr("10.0.0.1"), inet_addr("10.0.0.2"), 50);

    // Fetch info about the sender and receiver
    const auto client_search = get_client_info(inet_addr("10.0.0.1"));

    std::cout << "Request result: " << client_search.success << std::endl;
    if (client_search.success) {
        print_record(*client_search.record);
    }

    const auto client_search2 = get_client_info(inet_addr("10.0.0.2"));

    std::cout << "Request result: " << client_search2.success << std::endl;
    if (client_search2.success) {
        print_record(*client_search2.record);
    }
}

DbManager::~DbManager() {
    pthread_mutex_destroy(&database_access_lock);

    for (auto client_reader : client_locks) {
        pthread_mutex_destroy(client_reader.second);
        free(client_reader.second);
    }
    client_locks.clear();

    database_records.clear();
}

const db_response DbManager::get_client_info(in_addr_t client_ip) {
    try {
        // Client found, return success
        return {true, &database_records.at(client_ip),
                db_response::Description::SUCCESS};

    } catch (std::out_of_range e) {
        // Client not found, return failure
        return {false, {}, db_response::Description::NOT_FOUND};
    }
}

const db_response DbManager::register_client(in_addr_t client_ip) {
    // Generate new client template using provided ip
    const client_record new_client = {client_ip, STARTING_BALANCE,
                                      STARTING_REQUEST};

    // Check if the client already has a lock. Since a client gets a lock when
    // it is registered in the system, we are using it as a way to figure out if
    // the client is already registered or not, despite it being a little jank.
    try {
        auto client_lock = client_locks.at(client_ip);
    } catch (std::out_of_range) {
        // If it was not found
    }

    //  Try to insert new client
    auto insertion_result =
        database_records.insert(std::pair(client_ip, new_client));

    // Insertion result already operates like the behaviour defined for the
    // function, where first contains the iterator to the (ip,record) pair and
    // second contains the operation result
    const bool insertion_success = insertion_result.second;
    if (insertion_success) {
        // Create semaphores for the new client
        pthread_mutex_t *reader_mutex =
            (pthread_mutex_t *)malloc(sizeof(sem_t));

        pthread_mutex_init(reader_mutex, NULL);
        client_locks.insert(std::pair(client_ip, reader_mutex));

        return {true, &insertion_result.first->second,
                db_response::Description::SUCCESS};
    } else {
        return {false, &insertion_result.first->second,
                db_response::Description::NOT_FOUND};
    }
};

const db_response DbManager::make_transaction(in_addr_t sender_ip,
                                              in_addr_t receiver_ip,
                                              long transfer_amount) {
    // Make sure there is no one reading or writing on our client

    // Retrieve sender info
    client_record *sender;
    try {
        sender = &database_records.at(sender_ip);
    } catch (std::out_of_range e) {
        return {false, NULL, db_response::UNKNOWN_SENDER};
    }

    // Retrieve receiver_info
    client_record *receiver;
    try {
        receiver = &database_records.at(receiver_ip);
    } catch (std::out_of_range e) {
        return {false, sender, db_response::UNKNOWN_RECEIVER};
    }

    // Verify sender balance
    if (sender->balance < transfer_amount) {
        return {false, sender, db_response::INSUFFICIENT_BALANCE};
    }

    // Register transaction
    sender->balance -= transfer_amount;
    receiver->balance += transfer_amount;

    return {true, sender, db_response::SUCCESS};
}

void DbManager::print_record(client_record client) {
    struct in_addr temp_addr;
    temp_addr.s_addr = client.ip;
    std::cout << "#### Client " << inet_ntoa(temp_addr) << " ####" << std::endl;
    std::cout << "- Balance: " << client.balance << std::endl;
    std::cout << "- Last request: " << client.last_request << std::endl
              << std::endl;
}

} // namespace db_manager