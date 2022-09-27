#include "Client.h"
#include "../SR/SR_Receiver.h"

int sendBufferToSocket(char *buffer, int buffer_size, int sock_fd);

Client::Client(string client_conf_file_dir) {
    ifstream conf_file(client_conf_file_dir);
    if (conf_file) {
        conf_file >> server_ip_address >> server_port_number;
        conf_file >> client_port_number;
        conf_file >> requested_file_name;
        conf_file >> initial_window_size;
    } else {
        perror("Configuration File not found !!\n");
        exit(EXIT_FAILURE);
    }
}

void Client::init_client_socket() {
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
}

void Client::connect_to_server() {
    init_client_socket();
    memset(&serv_address, 0, sizeof(serv_address));

    // Filling server information
    serv_address.sin_family = AF_INET;
    serv_address.sin_port = htons(3000);
    serv_address.sin_addr.s_addr = INADDR_ANY;
    printf("client is ready.\n");

}

uint32_t Client::send_request_to_server() {
    printf("Start sending req.\n");
    // send the file name packet
    Packet packet = PacketHandler::create_packet(strdup(requested_file_name.c_str()), 0 , requested_file_name.size());
    Sender sender = Sender(serv_address);
    sender.send_packet(packet, sock_fd);
    printf("req sent.\n");
    /*
     * wait until receive ack from server
     */
    Ack_Server_Packet server_ack_packet =  Receiver::receive_ack_server_packet(sock_fd, serv_address);
    printf("ack received.\n");
    return server_ack_packet.packets_numbers;
}

void Client::receive_file(int strategy_option) {
    uint32_t number_of_packets = send_request_to_server();
    cout << "must receive " << number_of_packets << " packet" << endl;
    auto start = std::chrono::high_resolution_clock::now();
    // Process the packet according to the RED algorithm
    red(queue, buffer);

    // selective repeat
    SR_Receiver SR(sock_fd, requested_file_name, number_of_packets, serv_address);
    SR.recevFile();

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << " s\n";
    printf("Client finished\n");
}

/* sends buffer of chars over socket */
int sendBufferToSocket(char *buffer, int buffer_size, int sock_fd) {
    int sent = 0;
    const int TIMEOUT = 5;
    clock_t curTime = clock();
    // checking Timeout as socket may fail for many many times so we will stop trying to repeat.
    while (sent < buffer_size && (clock() - curTime) / CLOCKS_PER_SEC < TIMEOUT) {
        sent += send(sock_fd, (void *) (buffer + sent), buffer_size - sent, 0);
    }
    if (sent != buffer_size) {
        // then that means not all characters are sent because of timeout */
        return 0;
    }
    return 1;
}
