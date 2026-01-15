#include "requestHandler.hpp"
#include <string>
#include <iostream>
#include <stdexcept>
#include <vector>


// Message framing
/*
numberOfPayloadBytes|OperationCode|OperationMetadata|
- note the bytes in payload excludes the length of the numberOfBytes and the first | delimiter
ex: 31|Upload|FileSize:1496,name:1234|
ex: 16|ShowAllFolders||
*/


requestHandler::requestHandler(SSLServer &sslRef, pgConnection &pgRef) :
    ssl {sslRef}, pg {pgRef} {
        ssl.write ("Hello from Server!");
        std::cerr<< ssl.read () << std::endl;
    }

requestHandler::~requestHandler() {
    //nothing yet to deconstruct
}


std::vector<std::string> requestHandler::receiveMessage() {
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

std::string requestHandler::createMessage(std::string operation, std::string metadata) {

    std::string payload = operation + "|" + metadata + "|";

    size_t payloadSize = payload.size();
    std::string message = std::to_string(payloadSize) + "|" + payload;

    return message;
}
std::string requestHandler::pgDataToString (std::vector<std::vector<std::string>> data) {
    std::string outputString;
    for (size_t i = 0; i <data.size(); i ++) {
        for (size_t j = 0; j < data[i].size(); j ++) {
            outputString += data[i][j];
            outputString += ", ";
        }
        
        outputString += "\n";
    }

    return outputString;
}


std::string requestHandler::getFilesInFolder (std::string metadata) {
    //expected metadata in the form of "folderID:____"
    
    size_t pos = metadata.find(':'); 
    if (pos == std::string::npos) { 
        return "INVALID METADATA"; 
    }
    std::string folderID = metadata.substr(pos + 1);

    for (auto ch : folderID) {
        if (std::isdigit(static_cast<unsigned char>(ch)) == false) {
            return "INVALID METADATA";
        }
    }

    std::string query = "SELECT name FROM data WHERE parentfolder = " + folderID +";";

    std::vector<std::vector<std::string>> data = pg.sendQuery (query);

    std::string result = pgDataToString (data);

    return result;
}

std::string requestHandler::fileUpload(std::string metadata) {
    //parse metadata: name:____,folderID:____,size:____
    size_t pos = metadata.find(':'); 
    if (pos == std::string::npos) { 
        return "INVALID METADATA"; 
    }

    std::string name = metadata.substr(pos + 1, metadata.find(',') - (pos+1));

    metadata.erase(0, metadata.find(',')+1);

    pos = metadata.find(':'); 
    std::string folderID = metadata.substr(pos + 1, metadata.find(',') - (pos+1));

    metadata.erase(0, metadata.find(',') + 1);

    pos = metadata.find(':'); 
    size_t fileSize = std::strtoull(metadata.substr(pos + 1).c_str(), nullptr, 10); 
 
    size_t bytesRead = 0;

    pg.beginPGOperation();
    try {
        int fd = pg.createNewLargeObject(name, folderID, fileSize);

        sendMessage("Upload Ready","");

        while (bytesRead<fileSize) {
            std::string chunk = ssl.read();

            if (chunk.empty()) { 
                throw std::runtime_error("Client disconnected during upload"); 
            }

            bytesRead += chunk.size();
            pg.writeToLargeObject(chunk, fd);
        }

        pg.closeLargeObject(fd);

        pg.commitPGOperation();
    } catch (...){
        pg.rollbackPGOperation();
        throw;
    }
    return "Successful Upload";
}
    

void requestHandler::handlePayload(std::vector<std::string> payload) {
    std::string operation = payload[0];

    if (operation == "Get Files in Folder") {
        std::string result = getFilesInFolder (payload[1]);
        sendMessage("Result", result);
    } else if (operation == "File Upload") {
        std::string result = fileUpload (payload[1]);
        sendMessage ("Result", result);
    } else {
        sendMessage ("Invalid Operation", "");
    }

}


void requestHandler::sendMessage (std::string operation, std::string metadata) {
    std::string message = createMessage(operation, metadata);
    ssl.write(message);
}