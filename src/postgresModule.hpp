#pragma once

#include <string>
#include <memory>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <thread>
#include <fstream>


#include <postgresql/libpq-fe.h>
#include <postgresql/libpq/libpq-fs.h>


class pgConnection {
    private:
        PGconn* conn;
        std::vector <std::vector<std::string>> sendQuery (std::string query);
        void displayResponse (std::vector <std::vector<std::string>> data);
    public:
        pgConnection();
        ~pgConnection();
        int64_t insertFolder (std::string folderName,  int64_t parentID);
        void deleteFolder(int64_t id);
        void createNewFile (std::string fileName, std::string filePath, int64_t parentFolderID);
        void deleteFile (int64_t fileID);
};