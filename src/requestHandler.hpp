#pragma once

#include "openSSLModule.hpp"
#include "postgresModule.hpp"
#include <string>
#include <vector>

class requestHandler {
    private:
        SSLServer &ssl;
        pgConnection & pg;
        std::string createMessage(std::string operation, std::string metadata);
        std::string pgDataToString (std::vector<std::vector<std::string>> data);
        std::string getFilesInFolder (std::string metadata);
        std::string fileUpload(std::string metadata);
        std::string fileDownload (std::string metadata);
    public:
        requestHandler (SSLServer &sslRef, pgConnection &pgRef);
        ~requestHandler ();
        std::vector<std::string> receiveMessage();
        void sendMessage (std::string operation, std::string metadata);
        void handlePayload(std::vector<std::string> payload);
        
};