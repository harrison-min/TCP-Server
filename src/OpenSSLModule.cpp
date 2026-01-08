#include "OpenSSLModule.hpp"

class OpenSSLInitializer {
    public:
        OpenSSLInitializer() {
            int initResult = OPENSSL_init_ssl(0, nullptr);
            if (initResult == 0) {
                unsigned long error = ERR_get_error(); 
                std::string errorMessage = "OpenSSL init failed: ";
                errorMessage += ERR_error_string(error, nullptr);
                throw std::runtime_error(errorMessage);
            }

            std::cerr << "OpenSSL initialized successfully\n";
        }

        ~OpenSSLInitializer() {
            std::cerr << "OpenSSL cleaned up\n";
        }

        OpenSSLInitializer(const OpenSSLInitializer&) = delete;
        OpenSSLInitializer& operator=(const OpenSSLInitializer&) = delete;
};

static OpenSSLInitializer openssl;


//SSL Server implementation

void SSLServer::createSSLCTX () {
    ctx = SSL_CTX_new (TLS_server_method());
    if (ctx == nullptr) {
        unsigned long error = ERR_get_error(); 
        std::string errorMessage = "server SSL_CTX failed to generate: ";
        errorMessage += ERR_error_string(error, nullptr);
        throw std::runtime_error(errorMessage);
    }

    if (SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION) == 0) {
        SSL_CTX_free(ctx);
        ctx = nullptr;
        throw std::runtime_error( "Failed to set the minimum TLS protocol version");
    }
}

void SSLServer::loadCerts () {
    if (SSL_CTX_use_certificate_chain_file(ctx, "../certs/server_cert.pem") <= 0) {
        SSL_CTX_free(ctx);
        ctx = nullptr;
        throw std::runtime_error( "Failed to load the server certificate chain file");
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "../certs/server_key.pem", SSL_FILETYPE_PEM) <= 0) {
        SSL_CTX_free(ctx);
        ctx = nullptr;
        throw std::runtime_error( "Failed to load the server private key file");
    }

    if (SSL_CTX_check_private_key(ctx) == 0) {
        SSL_CTX_free(ctx);
        ctx = nullptr;
        throw std::runtime_error("Server private key does not match the certificate");
    }


    if (SSL_CTX_load_verify_locations(ctx, "../certs/client_cert.pem", nullptr) <= 0) {
        SSL_CTX_free(ctx);
        ctx = nullptr;
        throw std::runtime_error( "Failed to load the client verify file");
    }
}

SSLServer::SSLServer (): 
    ctx {nullptr} {
    createSSLCTX (); 
    //choose to set up options if needed      
    loadCerts();
    //choose to enable caching if needed
    SSL_CTX_set_verify (ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);

}

SSLServer::~SSLServer () {
    if (ctx != nullptr) {
        SSL_CTX_free(ctx);
    }
}




// SSL Client implementation


void SSLClient::createSSLCTX () {
    ctx = SSL_CTX_new (TLS_client_method());
    if (ctx == nullptr) {
        unsigned long error = ERR_get_error(); 
        std::string errorMessage = "client SSL_CTX failed to generate: ";
        errorMessage += ERR_error_string(error, nullptr);
        throw std::runtime_error(errorMessage);
    }

    if (SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION) == 0) {
        SSL_CTX_free(ctx);
        ctx = nullptr;
        throw std::runtime_error( "Failed to set the minimum TLS protocol version");
    }
}

void SSLClient::loadCerts () {
    if (SSL_CTX_use_certificate_chain_file(ctx, "../certs/client_cert.pem") <= 0) {
        SSL_CTX_free(ctx);
        ctx = nullptr;
        throw std::runtime_error( "Failed to load the client certificate chain file");
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "../certs/client_key.pem", SSL_FILETYPE_PEM) <= 0) {
        SSL_CTX_free(ctx);
        ctx = nullptr;
        throw std::runtime_error( "Failed to load the client private key file");
    }

    if (SSL_CTX_check_private_key(ctx) == 0) {
        SSL_CTX_free(ctx);
        ctx = nullptr;
        throw std::runtime_error("Client private key does not match the certificate");
    }


    if (SSL_CTX_load_verify_locations(ctx, "../certs/server_cert.pem", nullptr) <= 0) {
        SSL_CTX_free(ctx);
        ctx = nullptr;
        throw std::runtime_error( "Failed to load the server verify file");
    }
}


SSLClient::SSLClient (): 
    ctx {nullptr} {
    createSSLCTX (); 
    //choose to set up options if needed      
    loadCerts();
    //choose to enable caching if needed
    SSL_CTX_set_verify (ctx, SSL_VERIFY_PEER, nullptr);

}

SSLClient::~SSLClient () {
    if (ctx != nullptr) {
        SSL_CTX_free(ctx);
    }
}
