#pragma once
#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
using namespace std;

struct FAT32_BOOTSECTOR
{
    int byte_per_sector;
    int sector_per_cluster;
    int sector_before_FAT_table;
    int num_of_FAT_tables;
    int Volume_size;
    int sector_per_FAT;
    int first_cluster_of_RDET;
};
class FileSystemEntity {
protected:
    string name;
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
    int started_byte_bootsector;
    int started_byte_rdet;
    vector<FileSystemEntity*> rootDirectories;
    FAT32_BOOTSECTOR fat32bs;
public:
    void set_fat32_bootsector(int byte_per_sector, int sector_per_cluster, int sector_before_FAT_table, int num_of_FAT_tables, int Volume_size, int sector_per_FAT, int first_cluster_of_RDET)
    {
        this->fat32bs.byte_per_sector = byte_per_sector;
        this->fat32bs.sector_per_cluster = sector_per_cluster;
        this->fat32bs.sector_before_FAT_table = sector_before_FAT_table;
        this->fat32bs.num_of_FAT_tables = num_of_FAT_tables;
        this->fat32bs.Volume_size = Volume_size;
        this->fat32bs.sector_per_FAT = sector_per_FAT;
        this->fat32bs.first_cluster_of_RDET = first_cluster_of_RDET;
        this->started_byte_rdet = (sector_per_FAT * num_of_FAT_tables + sector_before_FAT_table) * byte_per_sector ;
    }
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
    void setStartedByte(int started_byte_bootsector)
    {
        this->started_byte_bootsector = started_byte_bootsector;
    }
    int getStartedByte() { return started_byte_bootsector; }
    int getStartedByteRDET() { return started_byte_rdet; }
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
int littleEndianByteArrayToInt(const BYTE* byteArray, size_t length);
void detectFormat(Computer& MyPC);
void GetRemovableDriveNames(Computer& MyPC);
void read_FAT32_BootSector(Computer& MyPC, int ith_drive, wstring drivePath);
void read_FAT32_Drives(Computer& MyPC);
void read_FAT32_RDET(Computer& MyPC, int ith_drive, wstring drivePath);
wstring stringToWideString(const string& str);
string createName(vector <vector <BYTE>> extra_entry, vector <BYTE> main_entry);