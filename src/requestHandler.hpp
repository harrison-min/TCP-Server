#pragma once

#include "openSSLModule.hpp"
#include "postgresModule.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
class requestHandler {
    private:
        SSLServer &ssl;
        pgConnection & pg;
        std::unordered_map <std::string, std::function<std::string (std::string)>> operationsMap;
        
        void initOperationsMap();

        std::string createMessage(std::string operation, std::string metadata);
        std::string pgDataToString (std::vector<std::vector<std::string>> data);

        //Operations
        std::string getFilesInFolder (std::string metadata);
        std::string fileUpload(std::string metadata);
        std::string fileDownload (std::string metadata);
        std::string deleteFile(std::string metadata);
        std::string createFolder (std::string metadata);
        std::string displayDirectory (std::string metadata);
        std::string openFolder (std::string metadata);
    public:
        requestHandler (SSLServer &sslRef, pgConnection &pgRef);
        ~requestHandler ();
        std::vector<std::string> receiveMessage();
        void sendMessage (std::string operation, std::string metadata);
        void handlePayload(std::vector<std::string> payload);
        
};