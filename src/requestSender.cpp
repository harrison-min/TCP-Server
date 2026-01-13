#include "requestSender.hpp"
#include "openSSLModule.hpp"
#include <iostream>
#include <string>

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

std::vector<std::string> requestSender::recieveMessage() {
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

    return {operation, metadata};
}

void requestSender::sendMessage (std::string operation, std::string metadata) {
    std::string message = createMessage(operation, metadata);
    ssl.write(message);
}

requestSender::requestSender(SSLClient & clientRef):
    ssl{clientRef} {
        std::cerr << ssl.read() << "\n";
        ssl.write("Hello from client!");
}

requestSender::~requestSender() {
    //nothing to deconstruct yet
}