

#include "db_manager.h"
#include "colors.h"
#include <iostream>

namespace db_manager {

DbManager *DbManager::get_instance() {
    static DbManager *instance = new DbManager();
    return instance;
}

DbManager::DbManager() {
    // The records are kept in an unordere map (indexed by their client_ip,
    // using a hash table  for efficient storage
    database_records = std::unordered_map<in_addr_t, client_record>();

    /// Initialize global acessor, used to indicate when a thead is in the
    /// process of acquiring locks
    pthread_mutex_init(&database_access_lock, {});

    // Client locks are used to avoid two threads trying to read or modify info
    // about two clients at the same time.
    client_locks = std::unordered_map<in_addr_t, pthread_mutex_t *>();

    total_transferred = 0;
    num_transactions = 0;
    total_balance = 0;
}

DbManager::~DbManager() {
    std::cout << RED << "[DATABASE] Destroying database instance!" << RESET
              << std::endl;
    pthread_mutex_destroy(&database_access_lock);

    for (auto client_reader : client_locks) {
        pthread_mutex_destroy(client_reader.second);
        free(client_reader.second);
    }
    client_locks.clear();

    database_records.clear();
}

const db_record_response DbManager::get_client_info(in_addr_t client_ip) {
    try {
        // Client found, return success
        return {true, database_records.at(client_ip),
                db_record_response::Description::SUCCESS};

    } catch (std::out_of_range const &) {
        // Client not found, return failure
        return {false, {}, db_record_response::Description::NOT_FOUND};
    }
}

const db_record_response DbManager::register_client(in_addr_t client_ip) {
    // Generate new client template using provided ip
    const client_record new_client = {client_ip, STARTING_BALANCE,
                                      STARTING_REQUEST};

    // Check if the client already has a lock. Since a client gets a lock when
    // it is registered in the system, we are using it as a way to figure out if
    // the client is already registered or not, despite it being a little jank.

    // Create a mutex for the new client
    pthread_mutex_t *client_mutex = (pthread_mutex_t *)malloc(sizeof(sem_t));
    pthread_mutex_init(client_mutex, {});
    // Lock the database since we are going to access the client locks
    lock_database();

    // Try to insert it into the mutex dictionary
    auto mutex_creation_result =
        client_locks.insert(std::pair(client_ip, client_mutex));

    // Lock the client since we are going to do operations on it
    lock_client(client_ip);

    // Finished client locks opertions
    unlock_database();

    // If it was already inserted, return a failure
    const bool mutex_insertion_success = mutex_creation_result.second;
    if (!mutex_insertion_success) {
        // Destroy mutex
        pthread_mutex_destroy(client_mutex);
        free(client_mutex);
        unlock_client(client_ip);
        return {false, {}, db_record_response::Description::DUPLICATE_IP};
    }

    //  Try to insert new client
    auto insertion_result =
        database_records.insert(std::pair(client_ip, new_client));

    // Insertion result already operates like the behaviour defined for the
    // function, where first contains the iterator to the client and second
    // contains the operation result
    const bool insertion_success = insertion_result.second;
    if (!insertion_success) {
        // This should never ever happen (mutex creation should cover client ip
        // duplication)]
        std::cerr << "[DATABASE ERROR]: Client insertion failed after mutex "
                     "insertion succeded!"
                  << std::endl;

        // Destroy mutex
        pthread_mutex_destroy(client_mutex);
        free(client_mutex);
        unlock_client(client_ip);

        return {false, {}, db_record_response::Description::DUPLICATE_IP};
    }

    unlock_client(client_ip);

    lock_database();
    // Add client balance to global balance
    total_balance += STARTING_BALANCE;
    unlock_database();

    return {true, insertion_result.first->second,
            db_record_response::Description::SUCCESS};
};

const db_record_response
DbManager::make_transaction(in_addr_t sender_ip, in_addr_t receiver_ip,
                            unsigned long int transfer_amount) {
    // Make sure there is no one reading or writing on our clients
    // Get locks
    lock_database();
    try {
        lock_client(sender_ip);
    } catch (std::out_of_range const &) {
        unlock_database();
        return {false, {}, db_record_response::UNKNOWN_SENDER};
    }

    try {
        lock_client(receiver_ip);
    } catch (std::out_of_range const &) {
        const auto sender_info = get_client_info(sender_ip);
        if (!sender_info.success) {
            std::cerr
                << "[DATABASE ERROR]: Client retrieval failed after mutex "
                   "acquisition succeded!"
                << std::endl;
        }
        unlock_client(sender_ip);
        unlock_database();
        return {false, sender_info.record,
                db_record_response::UNKNOWN_RECEIVER};
    }

    // Finished getting the locks
    unlock_database();

    // Retrieve sender info
    client_record *sender;
    try {
        sender = &database_records.at(sender_ip);
    } catch (std::out_of_range const &) {
        // This should never fail, since we were able to get the lock to the
        // sender, therefore it should exists in the records.
        unlock_client(sender_ip);
        unlock_client(receiver_ip);
        std::cerr << "[DATABASE ERROR]: Client retrieval failed after mutex "
                     "acquisition succeded!"
                  << std::endl;
        return {false, {}, db_record_response::UNKNOWN_SENDER};
    }

    // Retrieve receiver_info
    client_record *receiver;
    try {
        receiver = &database_records.at(receiver_ip);
    } catch (std::out_of_range const &) {
        // This should never fail, since we were able to get the lock to the
        // receiver, therefore it should exists in the records.
        unlock_client(sender_ip);
        unlock_client(receiver_ip);
        std::cerr << "[DATABASE ERROR]: Client retrieval failed after mutex "
                     "acquisition succeded!"
                  << std::endl;
        return {false, *sender, db_record_response::UNKNOWN_RECEIVER};
    }

    // Always increase request index
    sender->last_request++;

    // Verify sender balance
    if (sender->balance < transfer_amount) {
        return {false, *sender, db_record_response::INSUFFICIENT_BALANCE};
    }

    // Register transaction
    sender->balance -= transfer_amount;
    receiver->balance += transfer_amount;

    // Update global records
    lock_database();
    total_transferred += transfer_amount;
    num_transactions++;
    unlock_database();

    unlock_client(sender_ip);
    unlock_client(receiver_ip);

    return {true, *sender, db_record_response::SUCCESS};
}

const db_record_response DbManager::remove_client(in_addr_t client_ip) {
    // Get lock to the client
    lock_database();

    try {
        lock_client(client_ip);
    } catch (std::out_of_range const &) {
        unlock_database();
        return {false, {}, db_record_response::NOT_FOUND};
    }

    // Finished getting the lock
    unlock_database();

    int client_balance;
    try {
        client_balance = database_records.at(client_ip).balance;
    } catch (std::out_of_range const &) {
        unlock_client(client_ip);
        std::cerr << "[DATABASE ERROR]: Client search failed after mutex "
                     "acquisition succeded!"
                  << std::endl;
        return {false, {}, db_record_response::NOT_FOUND};
    }

    // Remove the client from the database
    auto removed_client_amout = database_records.erase(client_ip);
    if (removed_client_amout == 0) {
        std::cerr << "[DATABASE ERROR]: Client removal failed after mutex "
                     "acquisition succeded!"
                  << std::endl;
        unlock_client(client_ip);
        return {false, {}, db_record_response::NOT_FOUND};
    }

    lock_database();
    unlock_client(client_ip);
    pthread_mutex_destroy(client_locks.at(client_ip));

    auto removed_lock_amount = client_locks.erase(client_ip);
    unlock_database();

    if (removed_lock_amount == 0) {
        std::cerr << "[DATABASE ERROR]: Client lock removal failed after mutex "
                     "acquisition succeded!"
                  << std::endl;
        return {false, {}, db_record_response::NOT_FOUND};
    }

    lock_database();
    // Update global balance
    total_balance -= client_balance;
    unlock_database();

    return {true, {}, db_record_response::SUCCESS

    };
}

const db_metadata DbManager::get_db_metadata() {
    return {num_transactions, total_transferred, num_transactions};
}

void DbManager::print_record(client_record client) {
    struct in_addr temp_addr;
    temp_addr.s_addr = client.ip;
    std::cout << BOLD << "#### Client " << inet_ntoa(temp_addr) << " ####"
              << RESET << std::endl;
    std::cout << CYAN << "- Balance: " << client.balance << std::endl;
    std::cout << "- Last request: " << client.last_request << std::endl
              << RESET << std::endl;
}

void DbManager::lock_database() { pthread_mutex_lock(&database_access_lock); }

void DbManager::unlock_database() {
    pthread_mutex_unlock(&database_access_lock);
}

void DbManager::lock_client(in_addr_t client_ip) {
    pthread_mutex_lock(client_locks.at(client_ip));
}

void DbManager::unlock_client(in_addr_t client_ip) {
    try {
        pthread_mutex_unlock(client_locks.at(client_ip));
    } catch (std::out_of_range const &) {
        std::cerr << "[DATABASE ERROR]: Failed to unlock a client mutex after "
                     "it has presumably been locked! The system is probably in "
                     "a critical state!"
                  << std::endl;
    }
}

} // namespace db_manager