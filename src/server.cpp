#include "openSSLModule.hpp"
#include "postgresModule.hpp"
#include "requestHandler.hpp"
#include <iostream>

int main() {
    pgConnection myPostgres;
    SSLServer myServer;
    requestHandler handler (myServer, myPostgres);
    // myPostgres.deleteFile(9);
    // std::vector<std::string> payload = handler.receiveMessage();
    // handler.handlePayload(payload);
    std::vector<std::string> payload = handler.receiveMessage();
    handler.handlePayload(payload);
    return 0;
}