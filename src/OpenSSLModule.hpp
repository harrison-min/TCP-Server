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
        BIO * bio;
        void createSSLCTX();
        void loadCerts();
        void connectBIO();
    public: 
        SSLClient();
        ~SSLClient();
        std::string read();
        void write(const std::string &msg);
};

class SSLServer {
    private: 
        SSL_CTX * ctx;
        BIO * acceptorBIO;
        BIO * clientBIO;
        SSL * ssl;
        void createSSLCTX();
        void loadCerts();
        void createBIO();
        void createSSL();
        
    public:
        SSLServer();
        ~SSLServer();
        std::string read();
        void write(const std::string &msg);
};