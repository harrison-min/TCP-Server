#pragma once
#include "requestSender.hpp"
#include <string>
#include <exception>
#include <unordered_map>
#include <functional>

class clientUI {
    private:
        requestSender & request;
        std::unordered_map <std::string, std::function<void()>> mainMenuOptions;
        bool exitStatus;
        void displayMainMenu();
        void parseInput(std::string input);
        void initMenuOptions ();

        void fileDownload();
        void fileUpload();
        void setExitStatusTrue();
        void deleteFile();
        void createFolder();
        void openFolder();
        

    public:
        void run ();
        clientUI (requestSender & requestRef);
        ~clientUI ();    
};