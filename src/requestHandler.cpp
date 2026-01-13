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


std::vector<std::string> requestHandler::recieveMessage() {
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

    std::cerr<< "Recieved operation: " + operation << std::endl;
    std::cerr<< "Recieved metadata: " + metadata << std::endl;

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
    std::string folderID = metadata.substr(metadata.find(':') + 1);

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


void requestHandler::handlePayload(std::vector<std::string> payload) {
    std::string operation = payload[0];

    if (operation == "Get Files in Folder") {
        std::string result = getFilesInFolder (payload[1]);
        sendMessage("Result: ", result);
    } else {
        sendMessage ("Invalid Operation", "");
    }

}


void requestHandler::sendMessage (std::string operation, std::string metadata) {
    std::string message = createMessage(operation, metadata);
    ssl.write(message);
}