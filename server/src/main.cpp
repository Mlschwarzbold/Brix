#include "colors.h"
#include "date_time_utils.h"
#include "db_manager/db_manager.h"
#include "db_synchronizer/db_synchronizer.h"
#include "greeter/server_UDP_greeter.h"
#include "multiplexer/packet_multiplexer.h"
#include "election/heartbeat.h"
#include "election/redundancy_manager.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

void print_startup_message(db_manager::db_metadata db_metadata);

int main(int argc, char *argv[]) {
#if _DEBUG
    std::cout << BOLD << YELLOW << "Running debug build!" << RESET << std::endl;
#endif
    // Validação dos argumentos da linha de comando
    if (argc < 2) {
        std::cerr << "Uso: ./servidor <porta>" << std::endl;
        return 1;
    }
    // recebe e valida porta do servidor
    int port;
    try {
        port = std::stoi(argv[1]);
    } catch (const std::invalid_argument &e) {
        std::cerr << "Porta inválida: " << argv[1] << std::endl;
        return 1;
    }
    if (port < 0 || port > 65535) {
        std::cerr << "Valores válidos: 0 e 65535" << std::endl;
        return 1;
    }

    db_manager::DbManager *db = db_manager::DbManager::get_instance();

    db_synchronizer::DB_Synchronizer::setup(port);

    // Porta válida, iniciar servidor
    std::cout << YELLOW << "Servidor iniciado na porta: " << port << RESET
              << std::endl;

    auto db_metadata = db->get_db_metadata();

    print_startup_message(db_metadata);

    pthread_t greeter_thread, multiplexer_thread, synchronizer_thread, heartbeat_thread;

    pthread_create(&greeter_thread, NULL, udp_server_greeter::start_server,
                   (void *)&port);

    pthread_create(&multiplexer_thread, NULL,
                   multiplexer::start_multiplexer_server, (void *)&port);

    pthread_create(&synchronizer_thread, NULL, db_synchronizer::start_server,
                   nullptr);


    pthread_create(&heartbeat_thread, NULL, election::start_heartbeat_receiver, (void *)&port);


    election::RedundancyManager::get_instance();
        
    //election::heartbeat_test("122.0.0.2", port + 2, 1000, 3);

    pthread_join(greeter_thread, NULL);
    pthread_join(multiplexer_thread, NULL);
    pthread_join(heartbeat_thread, NULL);



    delete db;
    return 0;
}

void print_startup_message(db_manager::db_metadata db_metadata) {
    std::cout << YELLOW << getCurrentDateString() << " "
              << getCurrentTimeString() << " num_transactions "
              << db_metadata.num_transactions << " total_transferred "
              << db_metadata.total_transferred << " total_balance "
              << db_metadata.total_balance << RESET << std::endl;
}