#include <iostream>
#include "Server/Server.h"
using namespace std;

int main() {
    Server server("server.txt");
    int selective_repeat = 1;
    server.start_server(selective_repeat);
    return 0;
}