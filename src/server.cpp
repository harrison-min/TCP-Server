#include "OpenSSLModule.hpp"

int main() {
    SSLServer myServer;
    myServer.write("Hello from server!");
    std::cerr<< myServer.read()<<std::endl;


    return 0;

}