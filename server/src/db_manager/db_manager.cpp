

#include "db_manager.h"

namespace db_manager {

// client_record boilerplate

bool operator==(const client_record &record1, const client_record &record2) {
    return record1.client_ip == record2.client_ip;
}

bool operator<(const client_record &record1, const client_record &record2) {
    return record1.client_ip < record2.client_ip;
}

// The records are kept in a set (indexed by their client_ip), effectively using
// a red-black tree for efficient storage
std::set<client_record> database_records;

const db_response get_client_info(in_addr_t client_ip) {
    const client_record temp_record = {client_ip};

    const auto found_object = database_records.find(temp_record);

    if (found_object == database_records.end()) {
        return {false, {}, db_response::Description::NOT_FOUND};
    }

    return {true, found_object, db_response::Description::SUCCESS};
}

const db_response register_client(in_addr_t client_ip) {
    // Generate new client template using provided ip
    client_record new_client = {client_ip, STARTING_BALANCE, STARTING_REQUEST};

    // Try to insert new client
    auto insertion_result = database_records.insert(new_client);

    // Insertion result already operates like the behaviour defined for the
    // function, where first contains the iterator to the client and second
    // contains the operation result
    const bool insertion_success = insertion_result.second;
    if (insertion_success) {
        return {true, insertion_result.first,
                db_response::Description::SUCCESS};
    } else {
        return {false, insertion_result.first,
                db_response::Description::NOT_FOUND};
    }
};

void print_record(client_record record) {
    struct in_addr temp_addr;
    temp_addr.s_addr = record.client_ip;
    std::cout << "#### Client " << inet_ntoa(temp_addr) << " ####" << std::endl;
    std::cout << "- Balance: " << record.client_balance << std::endl;
    std::cout << "- Last request: " << record.last_request << std::endl
              << std::endl;
}

void demo() {
    database_records = std::set<client_record>();

    database_records.insert((client_record){inet_addr("10.0.0.1"), 200, 1});

    auto foo =
        database_records.insert((client_record){inet_addr("10.0.0.1"), 300, 2});

    print_record(*foo.first);

    const auto r1 = get_client_info(inet_addr("10.0.0.2"));

    std::cout << "Request result: " << r1.success << std::endl;
    if (r1.success) {
        print_record(*r1.result);
    }
}

} // namespace db_manager