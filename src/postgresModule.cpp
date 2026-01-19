#include "postgresModule.hpp"
#include <iostream>
#include <stdexcept>
#include <fstream>

pgConnection::pgConnection():
    conn {nullptr} {
    const char* user = std::getenv("POSTGRES_USER"); 
    const char* password = std::getenv("POSTGRES_PASSWORD");
    const char* dbname = std::getenv("POSTGRES_DB");
    const char* host = std::getenv("POSTGRES_HOST");

    std::string conninfo = "user=" + std::string(user) + " password=" + std::string(password) + " dbname=" + std::string(dbname) + " host=" + std::string(host);
    conn = PQconnectdb(conninfo.data());
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << PQerrorMessage(conn);
        throw std::runtime_error("Connection to postgres failed");
    }

    const char* name = PQdb(conn); 
    std::cerr << "Connected to database: " <<  name << std::endl;
}
pgConnection::~pgConnection() {
    if (conn!= nullptr) {
        PQfinish(conn);
    }
}

std::vector <std::vector<std::string>> pgConnection::sendQuery (std::string query) {
    PGresult * result = PQexec (conn, query.data());

    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        std::string error = PQerrorMessage(conn);
        PQclear (result);
        throw std::runtime_error ("Request failed: " + error);
    }

    int totalRows = PQntuples(result);
    int totalColumns = PQnfields(result);

    std::vector <std::vector<std::string>> data;
    data.resize(totalRows);

    for (size_t row = 0; row<totalRows; row ++) {

        data[row].resize(totalColumns);

        for (size_t column = 0; column< totalColumns; column ++) {
            std::string cellValue = PQgetvalue (result, row, column);
            data[row][column] = cellValue;
        }

    }
    PQclear(result);

    return data;
}

void pgConnection::displayResponse (std::vector <std::vector<std::string>> data) {
    for (size_t i = 0; i <data.size(); i ++) {
        for (size_t j = 0; j < data[i].size(); j ++) {
            std::cerr << data[i][j]<< "\t";
        }
        std::cerr << std::endl;
    }
}

void pgConnection::insertFolder (std::string folderName, std::string parentID) {
    std::string query = "INSERT INTO folder (name, parentFolder) VALUES ('" +  folderName + "', " + parentID + ") RETURNING id;";
    
    std::vector<std::vector<std::string>> response = sendQuery(query);
    if (response.empty() || response [0].empty()) {
        throw std::runtime_error ("SELECT response was empty");
    }

}

void pgConnection::deleteFolder(std::string id) {

    std::string query = "DELETE FROM folder WHERE id =" + id +" RETURNING id;";

    std::vector<std::vector<std::string>> response = sendQuery(query);
    displayResponse(response); 
}

int pgConnection::createNewLargeObject (std::string fileName, std::string parentFolderID, size_t fileSize) {
    Oid newOID = lo_create(conn, InvalidOid);
    if (newOID == 0) {
        throw std::runtime_error("Failed to create a new large object");
    }

    int loDescriptor = lo_open(conn, newOID, INV_WRITE);
    if (loDescriptor<0) {
        throw std::runtime_error("Failed to open large object");
    }

    std::string query = "INSERT INTO data (loID, parentFolder, size, name) VALUES (" +
            std::to_string(newOID) + ", " + parentFolderID + ", " + 
            std::to_string(fileSize) + ", '" + fileName + "') RETURNING id;";
        
    std::vector<std::vector<std::string>> response = sendQuery(query);
    displayResponse(response);

    return loDescriptor;
}

void pgConnection::writeToLargeObject (std::string & buffer, int loDescriptor) {
    size_t totalBytesWritten = 0;
    
    while (totalBytesWritten<buffer.size()) {
        int writtenBytes = lo_write(conn, loDescriptor, buffer.data()+totalBytesWritten, buffer.size()-totalBytesWritten);
        if (writtenBytes< 0) {
            throw std::runtime_error("Failed to write to Large object");
        }
        totalBytesWritten += writtenBytes;
    }
}

void pgConnection::closeLargeObject (int loDescriptor) {
    lo_close(conn, loDescriptor);
}

void pgConnection::beginPGOperation () {

    PQexec(conn, "BEGIN");
}

void pgConnection::commitPGOperation () {
    PQexec(conn, "COMMIT");
}

void pgConnection::rollbackPGOperation() {
    PQexec(conn, "ROLLBACK");
}

int pgConnection::openLOForReading (std::string loID) {

    int loDescriptor = lo_open(conn, static_cast<uint32_t>(std::stoul(loID)), INV_READ);
    if (loDescriptor < 0) {
        throw std::runtime_error("Failed to open large object");
    }

    return loDescriptor;
}

std::string pgConnection::readChunkFromLO (int fd) {
    size_t bufferSize = 16384;
    char buffer [bufferSize];
    
    int bytesRead = lo_read(conn, fd, buffer, bufferSize);
    if (bytesRead < 0) {
        throw std::runtime_error("Failed to read large object");
    }
    
    return std::string{buffer, static_cast<size_t>(bytesRead)};
}

void pgConnection::deleteFile (int64_t fileID) {
    std::string query = "SELECT loid FROM data WHERE id = " + std::to_string(fileID) + ";";

    std::vector <std::vector<std::string>> response = sendQuery (query);
    if (response.empty() || response [0].empty()) {
        throw std::runtime_error ("SELECT response was empty");
    }

    Oid loID = static_cast<Oid>(std::stoul(response[0][0]));

    PQexec(conn, "BEGIN");
    try {
        int result =  lo_unlink(conn, loID);
        if (result <0) {
            throw std::runtime_error ("Large Object unlinking was unsuccessful");
        }

        query = "DELETE FROM data WHERE id =" + std::to_string(fileID) + " RETURNING id;";
        response = sendQuery (query);
        if (response.empty() || response [0].empty()) {
            throw std::runtime_error ("DELETE Response was empty");
        }

        PQexec(conn, "COMMIT");
    } catch (...){
        PQexec(conn, "ROLLBACK"); 
        throw;
    }
}