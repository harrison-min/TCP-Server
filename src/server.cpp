#include "openSSLModule.hpp"
#include "postgresModule.hpp"
#include "requestHandler.hpp"
#include <iostream>
#include <exception>

int main() {
    pgConnection myPostgres;
    while (true) {
        try {
            SSLServer myServer;
            requestHandler handler (myServer, myPostgres);
            while (true) {
                std::vector<std::string> payload = handler.receiveMessage();
                handler.handlePayload(payload);
            }
        } catch (std::runtime_error) {
            std::cerr << "Restarting SSLServer\n";
        }
    }

    return 0;
}