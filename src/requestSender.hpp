#pragma once

#include "openSSLModule.hpp"
#include <string>
#include <vector>

class requestSender {
    private:
        SSLClient & ssl;
        std::string createMessage(std::string operation, std::string metadata);
    public:
        requestSender(SSLClient & clientRef);
        ~requestSender();
        std::vector<std::string> recieveMessage();
        void sendMessage (std::string operation, std::string metadata);
};