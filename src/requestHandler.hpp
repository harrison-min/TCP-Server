#pragma once

#include "openSSLModule.hpp"
#include "postgresModule.hpp"

class requestHandler {
    private:
        SSLServer &ssl;
        pgConnection & pg;
    public:
        requestHandler (SSLServer &sslRef, pgConnection &pgRef);
        ~requestHandler ();
};