#pragma once

#include "openSSLModule.hpp"
#include <string>
#include <vector>

class requestSender {
    private:
        SSLClient & ssl;
        std::string createMessage(std::string operation, std::string metadata);
        void sendMessage (std::string operation, std::string metadata);
        std::vector<std::string> receiveMessage();
        void displayFolderData (std::string data);
    public:
        requestSender(SSLClient & clientRef);
        ~requestSender();
        void uploadData (std::string filePath, std::string fileName, std::string targetFolder);
        void downloadData (std::string destinationPath, std::string fileName, std::string parentFolder);
        void deleteFile (std::string fileName, std::string parentFolder);
        void createFolder (std::string folderName, std::string parentFolder);
        void openFolder (std::string folderID);
};