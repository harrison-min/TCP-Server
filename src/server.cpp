#include <string>
#include <memory>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <thread>
#include <fstream>

//openssl
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

//socket
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    printf ("\n File Descriptor: %d\n", serverSocket);


    // specifying the address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

    // binding socket.
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // listening to the assigned socket
    listen(serverSocket, 5);

    // accepting connection request
    int clientSocket = accept(serverSocket, nullptr, nullptr);

    // recieving data
    char buffer[1024] = { 0 };
    recv(clientSocket, buffer, sizeof(buffer), 0);
    std::cout << "Message from client: " << buffer << std::endl;

    return 0;

}