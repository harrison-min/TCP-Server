#include "openSSLModule.hpp"
#include "postgresModule.hpp"
#include "requestHandler.hpp"
#include <iostream>

int main() {
    SSLServer myServer;
    pgConnection myPostgres;
    requestHandler handler (myServer, myPostgres);
    
    std::vector<std::string> message = handler.recieveMessage();

    std::cerr<< "Operation: " << message[0] << std::endl;
    std::cerr <<"Metadata: " << message[1] << std::endl; 

    handler.sendMessage("MessageRecieved", "");

    return 0;
}