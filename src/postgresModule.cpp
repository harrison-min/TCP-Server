#include "postgresModule.hpp"


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

int64_t pgConnection::insertFolder (std::string folderName,  int64_t parentID) {
    std::string queryParentID = "NULL";
    if (parentID>0) {
        queryParentID = std::to_string(parentID);
    }

    std::string query = "INSERT INTO folder (name, parentFolder) VALUES ('" +  folderName + "', " + queryParentID + ") RETURNING id;";
    
    std::vector<std::vector<std::string>> response = sendQuery(query);
    displayResponse(response); 

    return std::stoll(response[0][0]);
}

void pgConnection::deleteFolder(int64_t id) {

    std::string query = "DELETE FROM folder WHERE id =" + std::to_string(id) +" RETURNING id;";

    std::vector<std::vector<std::string>> response = sendQuery(query);
    displayResponse(response); 
}

void pgConnection::createNewFile (std::string fileName, std::string filePath, int64_t parentFolderID) {   
    PQexec(conn, "BEGIN");
    try {    
        Oid newOID = lo_create(conn, InvalidOid);
        if (newOID == 0) {
            throw std::runtime_error("Failed to create a new large object");
        }

        int loDescriptor = lo_open(conn, newOID, INV_WRITE);
        if (loDescriptor<0) {
            throw std::runtime_error("Failed to open large object");
        }

        int64_t fileSize = 0;
        int bufferSize = 4096;
        char buffer [bufferSize];

        std::ifstream fs (filePath, std::ios::binary);
        if (fs.is_open() == false) {
            throw std::runtime_error("Failed to open filePath");
        }

        while (true)  {
            fs.read(buffer, bufferSize);
            std::streamsize readBytes = fs.gcount();
            if (readBytes == 0) {
                break;
            }
            fileSize += readBytes;


            if (readBytes > 0) {
                int writtenBytes = lo_write(conn, loDescriptor, buffer, readBytes);
                if (writtenBytes< 0) {
                    throw std::runtime_error("Failed to write to Large object");
                }
            }
        }

        fs.close();
        lo_close(conn, loDescriptor);

        std::string query = "INSERT INTO data (loID, parentFolder, size, name) VALUES (" +
            std::to_string(newOID) + ", " + std::to_string(parentFolderID) + ", " + 
            std::to_string(fileSize) + ", '" + fileName + "') RETURNING id;";
        
        std::vector<std::vector<std::string>> response = sendQuery(query);
        displayResponse(response); 

        PQexec(conn, "COMMIT");
    } catch (...) {
        PQexec(conn, "ROLLBACK"); 
        throw;
    }
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