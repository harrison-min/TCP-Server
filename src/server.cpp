#include "openSSLModule.hpp"
#include "postgresModule.hpp"

int main() {
    pgConnection myPostgres;
    std::vector<std::vector<std::string>> data = myPostgres.getData("SELECT * FROM testTable");

    for (size_t i = 0; i <data.size(); i ++) {
        for (size_t j = 0; j < data[i].size(); j ++) {
            std::cerr << data[i][j]<< "\t";
        }
        std::cerr << std::endl;
    }

    SSLServer myServer;
    myServer.write("Hello from server!");
    std::cerr<< myServer.read();

    return 0;
}