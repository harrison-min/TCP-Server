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

std::vector <std::vector<std::string>> pgConnection::getData (std::string query) {
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