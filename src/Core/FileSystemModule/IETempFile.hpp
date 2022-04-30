#pragma once

// logging
#include "Core/LogModule/IELogger.hpp"

// std dependencies
#include <vector>
#include <string>
#include <fstream>


/**
 * A collection of data stored in RAM intended to be written to a file on disk when saved
 */
class IETempFile {
public:
    std::string name{}; // the filename
    std::string data{}; // the data in the file
    std::fstream fileIO{}; // fstream used to read and write to real files

    //Creates a TempFile with an associated real file
    IETempFile(std::string filename) {
        fileIO.open(filename);
        if(fileIO.is_open()) {
            data = readFileData();
            name = filename;
            std::cout << "Opened file";
        } else {
            std::cout << "File failed to open!";
        }
    }

    //change the file that this object is associated with
    void setAssociatedFile(std::string filePath) {
        fileIO.close();
        fileIO.open(filePath);
    }

private:

    //set the data variable to the real file's data
    std::string readFileData() {
        std::string content((std::istreambuf_iterator<char>(fileIO)),
                            (std::istreambuf_iterator<char>()));
        return content;
    }
};