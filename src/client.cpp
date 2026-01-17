#include "openSSLModule.hpp"
#include "requestSender.hpp"
#include "clientUI.hpp"
#include <iostream>

int main() {
    SSLClient myClient;
    requestSender sender (myClient);
    clientUI ui (sender);
    ui.run();
    
    return 0;
}