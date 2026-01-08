#pragma once

#include <string>
#include <memory>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <thread>
#include <fstream>

//openssl
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

//socket
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

class SSLClient {
    private: 
        SSL_CTX * ctx;
        void createSSLCTX();
        void loadCerts();
    public: 
        SSLClient();
        ~SSLClient();
};

class SSLServer {
    private: 
        SSL_CTX * ctx;
        void createSSLCTX();
        void loadCerts();
    public:
        SSLServer();
        ~SSLServer();
};