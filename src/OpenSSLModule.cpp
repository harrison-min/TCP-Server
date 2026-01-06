#include "OpenSSLModule.hpp"

class OpenSSLInitializer {
    public:
        OpenSSLInitializer() {
            SSL_library_init();
            OpenSSL_add_all_algorithms();
            SSL_load_error_strings();

            std::cerr << "OpenSSL initialized successfully\n";
        }

        ~OpenSSLInitializer() {
            ERR_free_strings();
            EVP_cleanup();
            std::cerr << "OpenSSL cleaned up\n";
        }

        OpenSSLInitializer(const OpenSSLInitializer&) = delete;
        OpenSSLInitializer& operator=(const OpenSSLInitializer&) = delete;
};

static OpenSSLInitializer openssl;