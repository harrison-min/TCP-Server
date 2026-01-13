#include "openSSLModule.hpp"
#include "postgresModule.hpp"
#include "requestHandler.hpp"
#include <iostream>

int main() {
    SSLServer myServer;
    pgConnection myPostgres;
    requestHandler handler (myServer, myPostgres);

    std::vector<std::string> payload = handler.recieveMessage();
    handler.handlePayload(payload);

    return 0;
}