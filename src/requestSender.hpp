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
        std::vector<std::string> receiveMessage();
        void sendMessage (std::string operation, std::string metadata);
        void uploadData (std::string filePath, std::string fileName, std::string targetFolder);
};