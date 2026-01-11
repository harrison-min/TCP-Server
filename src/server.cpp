#include "openSSLModule.hpp"
#include "postgresModule.hpp"
#include "requestHandler.hpp"

int main() {
    SSLServer myServer;
    pgConnection myPostgres;
    requestHandler handler (myServer, myPostgres);
    
    return 0;
}