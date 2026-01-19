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
        initOperationsMap();
    }

requestHandler::~requestHandler() {
    //nothing yet to deconstruct
}

void requestHandler::initOperationsMap() {
    operationsMap ["Get Files in Folder"] = [this](std::string arg) { return getFilesInFolder(arg); };
    operationsMap ["File Upload"] =         [this](std::string arg) { return fileUpload(arg); };
    operationsMap ["File Download"] =       [this](std::string arg) { return fileDownload(arg); };
    operationsMap ["Delete File"] =         [this](std::string arg) { return deleteFile(arg); };
    operationsMap ["Create Folder"] =       [this](std::string arg) { return createFolder(arg); };
    operationsMap ["Open Folder"] =         [this](std::string arg) { return openFolder(arg); };
    operationsMap ["Delete Folder"] =       [this](std::string arg) { return deleteFolder(arg); };
}

// ===========================================================
// Message Handling and Receiving 
// ===========================================================

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

void requestHandler::sendMessage (std::string operation, std::string metadata) {
    std::string message = createMessage(operation, metadata);
    ssl.write(message);
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
            outputString += ",";
        }
        
        outputString += "\n";
    }

    return outputString;
}

void requestHandler::handlePayload(std::vector<std::string> payload) {
    std::string operation = payload[0];
    std::string metadata = payload[1];
    try {
        std::string result = operationsMap.at(operation) (metadata);
        sendMessage("Result", result);
    } catch (std::out_of_range) {
        sendMessage ("Result", "Unrecognized Operation");
    } catch (std::runtime_error &err) {
        sendMessage ("Result", err.what());
    }
}

// ===========================================================
// Operations 
// ===========================================================
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
    
std::string requestHandler::fileDownload (std::string metadata) {
    //metadata framing:name:_____,parentfolder:_____
    size_t pos = metadata.find(':'); 

    std::string name = metadata.substr(pos + 1, metadata.find(',') - (pos+1));

    metadata.erase(0, metadata.find(',')+1);

    pos = metadata.find(':'); 
    std::string folderID = metadata.substr(pos + 1, metadata.find(',') - (pos+1));


    std::string query = "SELECT loID, size FROM data WHERE parentFolder =" + folderID + " AND name = '" + name + "';";
    std::vector<std::vector<std::string>> result = pg.sendQuery (query);

    std::string loID = result [0][0];
    std::string fileSizeString = result [0][1];

    sendMessage("Download Ready", fileSizeString);

    std::vector<std::string> response = receiveMessage();

    if (response [0] != "Download Ready") {
        return "Download failed";
    }

    size_t fileSize = std::stoull(fileSizeString.c_str());
    size_t totalBytesRead = 0;

    pg.beginPGOperation();
    try {
        int fd = pg.openLOForReading (loID);

        while (totalBytesRead < fileSize) {
            std::string chunk = pg.readChunkFromLO (fd);
            if (chunk.empty()) {
                break;
            }

            totalBytesRead += chunk.size();
            ssl.write(chunk);
        }

        pg.closeLargeObject (fd);
        pg.commitPGOperation();
    } catch (...){
        pg.rollbackPGOperation();
        throw;
    }
        
    return "Download Success!";
}

std::string requestHandler::deleteFile(std::string metadata) {
    //metadata framing:name:_____,parentfolder:_____

    size_t pos = metadata.find(':'); 

    std::string name = metadata.substr(pos + 1, metadata.find(',') - (pos+1));

    metadata.erase(0, metadata.find(',')+1);

    pos = metadata.find(':'); 
    std::string folderID = metadata.substr(pos + 1, metadata.find(',') - (pos+1));

    std::string query = "SELECT id FROM data WHERE parentFolder =" + folderID + " AND name = '" + name + "';";
    std::vector<std::vector<std::string>> result = pg.sendQuery (query);

    int64_t fileID = std::stoll (result[0][0]);
    try {    
        pg.deleteFile(fileID);
    } catch (...) {
        return "Deletion failed";
    }

    return "Deletion Success";
}

std::string requestHandler::createFolder(std::string metadata) {
    //metadata framing:name:_____,parentfolder:_____
    size_t pos = metadata.find(':'); 
    std::string name = metadata.substr(pos + 1, metadata.find(',') - (pos+1));
    metadata.erase(0, metadata.find(',')+1);
    pos = metadata.find(':'); 
    std::string folderID = metadata.substr(pos + 1, metadata.find(',') - (pos+1));

    try {    
        pg.insertFolder(name, folderID);
    } catch (...) {
        return "Folder Creation failed";
    }

    return "Folder Creation Success";
}

std::string requestHandler::deleteFolder(std::string metadata) {
    //metadata structure: folderID:___
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

    bool startedTransaction = false;

    if (pg.isTransactionIdle() == true) {
        pg.beginPGOperation();
        startedTransaction = true;
    }


    try {
        std::string fileQuery = "SELECT id FROM data WHERE parentFolder = " + folderID + ";";

        std::vector<std::vector<std::string>> fileResponse = pg.sendQuery (fileQuery);

        for (size_t index = 0; index < fileResponse.size(); index ++) {
            int64_t fileID = std::stoll (fileResponse[index][0]);
            pg.deleteFile(fileID);
        }

        std::string folderQuery = "SELECT id FROM folder WHERE parentFolder = " + folderID + ";";
        std::vector<std::vector<std::string>> folderResponse = pg.sendQuery (folderQuery);

        if (folderResponse.empty() == false) {
            for (size_t index = 0; index < folderResponse.size(); index ++) {
                deleteFolder("folderID:" + folderResponse[index][0]);
            }
        }
        
        pg.deleteFolderFromTable(folderID);

        if (startedTransaction == true) {
            pg.commitPGOperation();
        }

        return "Deletion Success";
    } catch (...) {
        if (startedTransaction == true) {
            pg.rollbackPGOperation();
        }
        throw;
    }

}

std::string requestHandler::openFolder (std::string metadata) {
    //metadata structure: folderID:___
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

    std::string folderQuery = "SELECT id, name FROM folder WHERE parentFolder = " + folderID + ";";
    std::vector<std::vector<std::string>> folderResponse = pg.sendQuery(folderQuery);

    std::string fileQuery = "SELECT id, name FROM data WHERE parentFolder = " + folderID + ";";
    std::vector<std::vector<std::string>> fileResponse = pg.sendQuery(fileQuery);


    //We concatenate the folder response and the file response by separating each field by a comma, each row by a newline
    // folders and files are separated with a :
    std::string result = "folders:" + pgDataToString(folderResponse) + "files:" + pgDataToString(fileResponse);

    return result;

}