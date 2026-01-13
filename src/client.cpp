#include "openSSLModule.hpp"
#include "requestSender.hpp"
#include <iostream>

int main() {
    SSLClient myClient;
    requestSender sender (myClient);
    sender.sendMessage("Get Files in Folder","folder:1");
    sender.recieveMessage();

    return 0;
}