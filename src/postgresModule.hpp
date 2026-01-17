#pragma once

#include <string>
#include <vector>


#include <postgresql/libpq-fe.h>
#include <postgresql/libpq/libpq-fs.h>


class pgConnection {
    private:
        PGconn* conn;

    public:
        pgConnection();
        ~pgConnection();
        std::vector <std::vector<std::string>> sendQuery (std::string query);
        void displayResponse (std::vector <std::vector<std::string>> data);
        void insertFolder (std::string folderName, std::string parentID);
        void deleteFolder(int64_t id);
        void deleteFile (int64_t fileID);
        int createNewLargeObject (std::string fileName, std::string parentFolderID, size_t fileSize);
        void writeToLargeObject (std::string & buffer, int loDescriptor);
        void closeLargeObject (int loDescriptor);
        void beginPGOperation (); 
        void commitPGOperation (); 
        void rollbackPGOperation(); 
        int openLOForReading (std::string loID);
        std::string readChunkFromLO (int fd);
};
