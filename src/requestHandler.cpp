#include "requestHandler.hpp"
#include <string>
#include <iostream>
#include <stdexcept>



requestHandler::requestHandler(SSLServer &sslRef, pgConnection &pgRef) :
    ssl {sslRef}, pg {pgRef} {
        ssl.write ("Hello from Server!");
        std::cerr<< ssl.read () << std::endl;
    }

requestHandler::~requestHandler() {
    //nothing yet to deconstruct
}