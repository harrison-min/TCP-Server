#include "clientUI.hpp"
#include "requestSender.hpp"
#include <iostream>
#include <unordered_map>

void clientUI::displayMainMenu() {
    std::cerr<< "MAIN MENU\n" 
    << "1) FILE DOWNLOAD\n" 
    << "2) FILE UPLOAD\n" 
    << "3) DELETE FILE\n"
    << "4) CREATE FOLDER\n"
    << "q) EXIT PROGRAM\n";
}

void clientUI::parseInput(std::string input) {
    try {
        mainMenuOptions.at(input)();
    } catch (std::out_of_range) {
        std::cerr<< "Input unrecognized, please try again\n";
    }  
}

void clientUI::run () {
    while (exitStatus == false) {
        std::cerr << "\033[2J\033[1;1H";
        displayMainMenu();
        std::string input; 
        std::getline (std::cin, input);
        parseInput (input);
    }
} 

void clientUI::initMenuOptions () {
    mainMenuOptions ["1"] = [this]() {fileDownload(); };
    mainMenuOptions ["2"] = [this]() {fileUpload(); };
    mainMenuOptions ["3"] = [this]() {deleteFile(); };
    mainMenuOptions ["4"] = [this]() {createFolder(); };
    mainMenuOptions ["q"] = [this]() {setExitStatusTrue(); };
}

void clientUI::createFolder() {
    std::cerr << "You selected create folder!\n";
    std::cerr << "Folder Name: ";
    std::string name; 
    std::getline (std::cin, name);

    std::cerr << "Parent Folder ID: ";
    std::string parentFolder; 
    std::getline (std::cin, parentFolder);
    
    request.createFolder(name, parentFolder);
}

void clientUI::deleteFile() {
    std::cerr << "You selected delete file!\n";
    std::cerr << "File Name: ";
    std::string name; 
    std::getline (std::cin, name);

    std::cerr << "Parent Folder ID: ";
    std::string parentFolder; 
    std::getline (std::cin, parentFolder);
    
    request.deleteFile(name, parentFolder);
}

void clientUI::fileDownload() {
    std::cerr << "You selected file download!\n";
    std::cerr << "File Name: ";
    std::string name; 
    std::getline (std::cin, name);

    std::cerr << "Parent Folder ID: ";
    std::string parentFolder; 
    std::getline (std::cin, parentFolder);
    
    std::cerr << "Destination Path: ";
    std::string destination; 
    std::getline (std::cin, destination);

    request.downloadData(destination, name, parentFolder);
}
void clientUI::fileUpload() {
    std::cerr<< "You selected file Upload!\n";
    std::cerr << "File Name: ";
    std::string name; 
    std::getline (std::cin, name);

    std::cerr << "File Path to file: ";
    std::string filePath; 
    std::getline (std::cin, filePath);
    
    std::cerr << "Target Folder ID: ";
    std::string folderID; 
    std::getline (std::cin, folderID);

    request.uploadData(filePath, name, folderID);
}

void clientUI::setExitStatusTrue() {
    exitStatus = true;
}

clientUI::clientUI(requestSender & requestRef) :
    request (requestRef), exitStatus (false) {
        initMenuOptions();
}

clientUI::~clientUI () {
    std::cerr<< "Goodbye!\n";
}