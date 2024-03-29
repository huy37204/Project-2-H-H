#pragma once
#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
using namespace std;
class FileSystemEntity {
protected:
    string name;
    string path;
    string attributes;
    string date_created;
    double total_size;

public:
    FileSystemEntity() = default;

    virtual ~FileSystemEntity() {}

    virtual void displayInfo() = 0; 


};


class File : public FileSystemEntity {
private:

public:
    File() = default;

    void displayInfo() {
   
    }


};


class Directory : public FileSystemEntity {
private:
    vector<FileSystemEntity*> contents;

public:
    Directory() = default;

    void addEntity(FileSystemEntity* entity) {
        contents.push_back(entity);
    }

    void displayInfo() {
    }
    ~Directory()
    {
        for (int i = 0; i < contents.size(); i++)
            delete contents[i];
    }
};

class Drive {
private:
    string name;
    string type;
    vector<FileSystemEntity*> rootDirectories;
public:
    void setName(string name)
    {
        this->name = name;
    }
    string getName() { return this->name; }
    void setType(string type)
    {
        this->type = type;
    }
    string getType() { return this->type; }
    ~Drive()
    {
        for (int i = 0; i < rootDirectories.size(); i++)
            delete rootDirectories[i];
    }
};

class Computer {
public:
    vector<Drive*> root_Drives;
    void addRootDrive(Drive*& d) {
        root_Drives.push_back(d);
    }

};
void detectFormat(Computer& MyPC);
void GetRemovableDriveNames(Computer& MyPC);