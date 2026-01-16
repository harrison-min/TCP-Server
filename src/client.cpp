#include "openSSLModule.hpp"
#include "requestSender.hpp"
#include <iostream>

int main() {
    SSLClient myClient;
    requestSender sender (myClient);
    // sender.uploadData("test/", "test.png", "1");
    sender.downloadData("target/", "test.png", "1");
    
    return 0;
}