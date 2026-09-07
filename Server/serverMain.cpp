#include <iostream>
#include "Server/Server.h"

#define QUEUE_CAPACITY 128
#define BUFFER_SIZE 1024
using namespace std;

int main() {
    Server server("server.txt");
    int selective_repeat = 1;

    Queue *queue = createQueue(QUEUE_CAPACITY);
    char buffer[BUFFER_SIZE] = {0};

    server.start_server(selective_repeat, queue, buffer);
    return 0;
}