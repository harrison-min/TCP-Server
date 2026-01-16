#include "requestSender.hpp"
#include "openSSLModule.hpp"
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>

// Message framing
/*
numberOfPayloadBytes|OperationCode|OperationMetadata|
- note the bytes in payload excludes the length of the numberOfBytes and the first | delimiter
ex: 31|Upload|FileSize:1496,name:1234|
ex: 16|ShowAllFolders||
*/

std::string requestSender::createMessage(std::string operation, std::string metadata) {
    std::string delimiter = "|";

    std::string payload = operation + delimiter + metadata + delimiter;

    size_t payloadSize = payload.size();
    std::string message = std::to_string(payloadSize) + delimiter + payload;

    return message;
}

std::vector<std::string> requestSender::receiveMessage() {
    std::string buffer;
    char delimiter = '|';
    while (buffer.find(delimiter) == std::string::npos) {
        buffer += ssl.read();
    }

    size_t payloadSize =  std::stoull(buffer.substr(0, buffer.find(delimiter)));

    buffer.erase(0, buffer.find(delimiter) + 1);

    size_t readPayloadBytes =buffer.size();

    while (readPayloadBytes < payloadSize) {
        buffer += ssl.read();
        readPayloadBytes = buffer.size();
    }

    std::string operation = buffer.substr (0, buffer.find(delimiter));
    buffer.erase (0, buffer.find(delimiter) + 1);
    std::string metadata = buffer.substr(0, buffer.find(delimiter));

    std::cerr<< "Received operation: " + operation << std::endl;
    std::cerr<< "Received metadata: " + metadata << std::endl;

    return {operation, metadata};
}

void requestSender::sendMessage (std::string operation, std::string metadata) {
    std::string message = createMessage(operation, metadata);
    ssl.write(message);
}

void requestSender::uploadData (std::string filePath, std::string fileName, std::string targetFolder) {
    //first send the initial message
    std::uintmax_t size = std::filesystem::file_size(filePath + fileName);

    sendMessage("File Upload", "name:" + fileName + ", folderID:" + targetFolder + ", size:" + std::to_string(size));

    //receive the response
    std::vector<std::string> response = receiveMessage();
    if (response[0] != "Upload Ready") {
        return;
    }

    //stream the data
    int bufferSize = 16384;
    char buffer [bufferSize];
    std::uintmax_t remaining = size;

    std::ifstream fs (filePath + fileName, std::ios::binary);
    if (fs.is_open() == false) {
        throw std::runtime_error("Failed to open filePath");
    }

    std::string outputBuffer;

    while (remaining>0)  {
        fs.read(buffer, bufferSize);
        std::streamsize readBytes = fs.gcount();
        if (readBytes <= 0) {
            break;
        }
        remaining -= readBytes;
        outputBuffer.assign(buffer,readBytes);
        ssl.write(outputBuffer);

    }
    fs.close();

    receiveMessage();
}

void requestSender::downloadData (std::string destinationPath, std::string fileName, std::string parentFolder) {
    sendMessage("File Download", "name:" + fileName + ", parentfolder:" + parentFolder);

    std::vector<std::string> response = receiveMessage();
    
    if (response [0] != "Download Ready") {
        return;
    }

    size_t fileSize = std::stoull(response[1].c_str());
    
    std::ofstream fs (destinationPath + fileName, std::ios::binary);
    if (fs.is_open() == false) {
        throw std::runtime_error("Failed to open file");
    }

    sendMessage("Download Ready", "");

    size_t remaining = fileSize;

    while (remaining>0)  {
        std::string chunk = ssl.read();

        fs.write(chunk.data(), chunk.size());
        if (!fs) { 
            throw std::runtime_error("File write failed"); 
        }
        remaining -= chunk.size();
    }
    fs.close();

    receiveMessage();

}

requestSender::requestSender(SSLClient & clientRef):
    ssl{clientRef} {
        std::cerr << ssl.read() << "\n";
        ssl.write("Hello from client!");
}

requestSender::~requestSender() {
    //nothing to deconstruct yet
}

