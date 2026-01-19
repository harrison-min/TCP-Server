#include "clientUI.hpp"
#include "requestSender.hpp"
#include <iostream>
#include <unordered_map>

clientUI::clientUI(requestSender & requestRef) :
    request (requestRef), exitStatus (false) {
        initMenuOptions();
}

clientUI::~clientUI () {
    std::cerr<< "Goodbye!\n";
}

void clientUI::initMenuOptions () {
    mainMenuOptions ["1"] = [this]() {fileDownload(); };
    mainMenuOptions ["2"] = [this]() {fileUpload(); };
    mainMenuOptions ["3"] = [this]() {deleteFile(); };
    mainMenuOptions ["4"] = [this]() {createFolder(); };
    mainMenuOptions ["5"] = [this]() {openFolder(); };
    mainMenuOptions ["6"] = [this]() {deleteFolder(); };
    mainMenuOptions ["q"] = [this]() {setExitStatusTrue(); };
}

// ===========================================================
// UI functions
// ===========================================================


void clientUI::displayMainMenu() {
    std::cerr<< "MAIN MENU\n" 
    << "1) FILE DOWNLOAD\n" 
    << "2) FILE UPLOAD\n" 
    << "3) DELETE FILE\n"
    << "4) CREATE FOLDER\n"
    << "5) OPEN FOLDER\n"
    << "6) DELETE FOLDER\n"
    << "q) EXIT PROGRAM\n";
}

void clientUI::parseInput(std::string input) {
    try {
        mainMenuOptions.at(input)();
    } catch (std::out_of_range) {
        std::cerr<< "Input unrecognized, please try again\n";
    } catch (std::runtime_error & error) {
        std::cerr<< "Operation failed due to runtime error: " << error.what() << "\n";
    }
}

void clientUI::run () {
    while (exitStatus == false) {
        //std::cerr << "\033[2J\033[1;1H"; Clears the screen completely
        displayMainMenu();
        std::string input; 
        std::getline (std::cin, input);
        parseInput (input);
    }
} 


// ===========================================================
// Operations 
// ===========================================================


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
    std::cerr<< "You selected file upload!\n";
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

void clientUI::openFolder() {
    std::cerr<< "You selected open folder!\n";
    std::cerr << "Parent Folder ID: ";
    std::string id; 
    std::getline (std::cin, id);

    request.openFolder(id);
}

void clientUI::deleteFolder() {
    std::cerr<< "You selected delete folder!\n";
    std::cerr << "Folder ID: ";
    std::string id; 
    std::getline (std::cin, id);

    request.deleteFolder(id);
}