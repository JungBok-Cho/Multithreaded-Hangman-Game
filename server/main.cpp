#include "server.h"
#include "game.h"
#include "connection.h"


int main() {
    int server_fd;
    struct sockaddr_in new_address;
    server myServer; // Server object

    // Set up the server
    myServer.serverSetup(new_address, server_fd);
    sleep(1);

    // Run the server
    do {
        myServer.createConnection(new_address, server_fd);
    } while(true);

    std::cout << "Shutting down server\n";

    return 1;
}
