#include "openSSLModule.hpp"
#include "postgresModule.hpp"

int main() {
    pgConnection myPostgres;

    int64_t newFolderID = myPostgres.insertFolder("myTestFolder", 1);
    myPostgres.deleteFolder(newFolderID);

    SSLServer myServer;
    myServer.write("Hello from server!");
    std::cerr<< myServer.read() <<std::endl;

    return 0;
}