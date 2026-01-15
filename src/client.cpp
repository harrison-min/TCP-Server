#include "openSSLModule.hpp"
#include "requestSender.hpp"
#include <iostream>

int main() {
    SSLClient myClient;
    requestSender sender (myClient);
    sender.uploadData("test/", "Docker Desktop Installer.exe", "1");
    return 0;
}