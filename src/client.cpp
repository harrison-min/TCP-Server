#include "openSSLModule.hpp"
#include "requestSender.hpp"
#include <iostream>

int main() {
    SSLClient myClient;
    requestSender sender (myClient);
    sender.sendMessage("testOperation","someMetadataHere");
    std::vector<std::string> message = sender.recieveMessage();

    std::cerr<< "Operation: " << message[0] << std::endl;
    std::cerr <<"Metadata: " << message[1] << std::endl; 

    
    return 0;
}