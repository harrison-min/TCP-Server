#pragma once

#include <string>
#include <memory>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <thread>
#include <fstream>


#include <postgresql/libpq-fe.h>


#include "postgresModule.hpp"

class pgConnection {
    private:
        PGconn* conn;
    public:
        pgConnection();
        ~pgConnection();
        std::vector <std::vector<std::string>> getData (std::string query);
};