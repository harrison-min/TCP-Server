#include "openSSLModule.hpp"

int main() {
    SSLClient myClient;
    std::cerr<< myClient.read();
    myClient.write("Hello from client!");
    return 0;
}