#include "openSSLModule.hpp"
#include <iostream>
#include <vector>
#include <stdexcept>


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

// ==================================================================================================================
//SSL Server implementation
// ==================================================================================================================
void SSLServer::createSSLCTX () {
    ctx = SSL_CTX_new (TLS_server_method());
    if (ctx == nullptr) {
        unsigned long error = ERR_get_error(); 
        std::string errorMessage = "server SSL_CTX failed to generate: ";
        errorMessage += ERR_error_string(error, nullptr);
        throw std::runtime_error(errorMessage);
    }

    if (SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION) == 0) {
        throw std::runtime_error( "Failed to set the minimum TLS protocol version");
    }
}

void SSLServer::loadCerts () {
    if (SSL_CTX_use_certificate_chain_file(ctx, "../certs/server_cert.pem") <= 0) {
        throw std::runtime_error( "Failed to load the server certificate chain file");
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "../certs/server_key_unencrypted.pem", SSL_FILETYPE_PEM) <= 0) {
        throw std::runtime_error( "Failed to load the server private key file");
    }

    if (SSL_CTX_check_private_key(ctx) == 0) {
        throw std::runtime_error("Server private key does not match the certificate");
    }

    if (SSL_CTX_load_verify_locations(ctx, "../certs/client_cert.pem", nullptr) <= 0) {
        throw std::runtime_error( "Failed to load the client verify file");
    }
}

void SSLServer::createBIO() {
    acceptorBIO = BIO_new_accept("12345");
    if (acceptorBIO == nullptr) {
        throw std::runtime_error( "Error creating server acceptor BIO");
    }
    
    BIO_set_bind_mode(acceptorBIO, BIO_BIND_REUSEADDR);

    // first call creates and binds socket to BIO object, 2nd call awaits connection
    if (BIO_do_accept(acceptorBIO) <= 0) {
        throw std::runtime_error( "Error setting up server accepter socket");
    }

    if (BIO_do_accept(acceptorBIO) <= 0) {
        throw std::runtime_error( "Error accepting the client");
    }

    clientBIO = BIO_pop(acceptorBIO);
    std::cerr<< "New client connection accepted"<< std::endl;
}

void SSLServer::createSSL () {
    ssl = SSL_new(ctx);
    if (ssl == nullptr) {
        throw std::runtime_error( "Error creating SSL handle for new connection");
    }

    SSL_set_bio(ssl, clientBIO, clientBIO);
    if (SSL_accept(ssl) <= 0) {
        
        throw std::runtime_error("Error performing SSL handshake with client");
    }
    std::cerr<< "SSL Handshake success!" <<std::endl;
}

SSLServer::SSLServer (): 
    ctx {nullptr}, acceptorBIO {nullptr}, clientBIO {nullptr}, ssl {nullptr} {
    createSSLCTX (); 
    //choose to set up options if needed      
    loadCerts();
    //choose to enable caching if needed
    SSL_CTX_set_verify (ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);

    createBIO();
    createSSL();
}

SSLServer::~SSLServer () {
    if (ssl != nullptr) {
        SSL_free (ssl);
    }
    if (ctx != nullptr) {
        SSL_CTX_free(ctx);
    }
    if (acceptorBIO != nullptr) {
        BIO_free (acceptorBIO);
    }
    
}

std::string SSLServer::read() {
    char buffer [4096] = {0};
    int n = SSL_read(ssl, buffer, sizeof(buffer));
    if (n<= 0) {
        throw std::runtime_error ("Server SSL read failed");
    }
    return std::string(buffer, n);
}

void SSLServer::write(const std::string &msg) {
    size_t totalBytesWritten =0;
    while(totalBytesWritten<msg.size()) {
        int n = SSL_write(ssl, msg.data() +totalBytesWritten, msg.size() - totalBytesWritten);
        if (n<= 0) {
            throw std::runtime_error ("Server SSL write failed");
        }
        totalBytesWritten += n;
    }
}

// ==================================================================================================================
// SSL Client implementation
// ==================================================================================================================

void SSLClient::createSSLCTX () {
    ctx = SSL_CTX_new (TLS_client_method());
    if (ctx == nullptr) {
        unsigned long error = ERR_get_error(); 
        std::string errorMessage = "client SSL_CTX failed to generate: ";
        errorMessage += ERR_error_string(error, nullptr);
        throw std::runtime_error(errorMessage);
    }

    if (SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION) == 0) {
        throw std::runtime_error( "Failed to set the minimum TLS protocol version");
    }
}

void SSLClient::loadCerts () {
    if (SSL_CTX_use_certificate_chain_file(ctx, "../certs/client_cert.pem") <= 0) {
        throw std::runtime_error( "Failed to load the client certificate chain file");
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "../certs/client_key.pem", SSL_FILETYPE_PEM) <= 0) {
        throw std::runtime_error( "Failed to load the client private key file");
    }

    if (SSL_CTX_check_private_key(ctx) == 0) {
        throw std::runtime_error("Client private key does not match the certificate");
    }


    if (SSL_CTX_load_verify_locations(ctx, "../certs/server_cert.pem", nullptr) <= 0) {
        throw std::runtime_error( "Failed to load the server verify file");
    }
}

void SSLClient::connectBIO() {
    bio = BIO_new_ssl_connect(ctx); 
    if (bio == nullptr) {
        throw std::runtime_error("Error in creating SSL BIO");
    }
    BIO_set_conn_hostname(bio, "tcp-server:12345");

    if(BIO_do_connect(bio) <= 0) {
       throw std::runtime_error ("Error in connecting to server");
    }

    if(BIO_do_handshake(bio) <= 0) {
        throw std::runtime_error("Error in performing handshake");
    }

    std::cerr<<"SSL Handshake success!" <<std::endl;
}

SSLClient::SSLClient (): 
    ctx {nullptr}, bio {nullptr}{
    createSSLCTX (); 
    //choose to set up options if needed      
    loadCerts();
    //choose to enable caching if needed
    SSL_CTX_set_verify (ctx, SSL_VERIFY_PEER, nullptr);
    connectBIO();    
}

SSLClient::~SSLClient () {
    if (bio != nullptr) {
        BIO_free_all(bio);
    }
    if (ctx != nullptr) {
        SSL_CTX_free(ctx);
    }
}

std::string SSLClient::read() {
    char buffer [4096] = {0};
    int n = BIO_read(bio, buffer, sizeof(buffer));
    if (n<=0) {
        throw std::runtime_error("Client BIO read failed");
    }
    return std::string(buffer, n);
}

void SSLClient::write(const std::string &msg) {
    size_t totalBytesWritten =0;
    while(totalBytesWritten<msg.size()) {
        int n = BIO_write (bio, msg.data () + totalBytesWritten, msg.size() - totalBytesWritten);
        if (n<= 0) {
            throw std::runtime_error("Client BIO write failed");
        }
        totalBytesWritten += n;
    }
}