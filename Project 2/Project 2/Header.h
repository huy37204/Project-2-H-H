#ifndef FILESYSTEMENTITY_H
#define FILESYSTEMENTITY_H
#pragma once
#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
using namespace std;

class Drive;

struct Date
{
    int day, month, year;
};
struct Time
{
    int hour, minute, second, milisecond;
};
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
    Date date_created;
    Time time_created;
    vector <int> cluster_pos;
    long long total_size;
    vector <vector<BYTE>>data;

public:
    FileSystemEntity() = default;

    void setName(string name) { this->name = name; }
    string getName() { return name; }


    void setTime(Time t) { this->time_created = t; }
    void setDate(Date d) { this->date_created = d; }
    
    void add_cluster_pos(int pos) { cluster_pos.push_back(pos); }
    int get_pos_cluster(int index) { return cluster_pos[index]; }

    void setTotalSize(int size) { this->total_size = size; }
    long long getTotalSize() { return total_size; }

    void pushData(vector<BYTE>DATA) { data.push_back(DATA); }

    void getClusterPos(vector <int>& vec)
    {
        vec = vector<int>{};
        for (int i = 0; i < cluster_pos.size(); i++)
        {
            vec.push_back(cluster_pos[i]);
        }
    }


    void read_next_sector(Drive* dr, wstring drivePath);

    virtual void displayInfo() {};
};


class File : public FileSystemEntity {
private:

public:
    File() = default;
    void displayInfo() {
        cout << "File: " << getName() << "; ";
        cout << "Date created: " << date_created.day << "/" << date_created.month << "/" << date_created.year << "; ";
        cout << "Time created: " << time_created.hour << ":" << time_created.minute << ":" << time_created.second << ":" << time_created.milisecond << "; ";
        cout << "Total size: " << total_size << "; ";
        cout << "Cluster pos: ";
        for (int i = 0; i < cluster_pos.size(); i++)
        {
            cout << cluster_pos[i] << "; ";
        }
        cout << endl;
    }
    void read_next_sector(Drive* dr, wstring drivePath)
    {
        FileSystemEntity::read_next_sector(dr, drivePath);
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
        cout << "Directory: " << getName() << "; ";
        cout << "Date created: " << date_created.day << "/" << date_created.month << "/" << date_created.year << "; ";
        cout << "Time created: " << time_created.hour << ":" << time_created.minute << ":" << time_created.second << ":" << time_created.milisecond << "; ";
        cout << "Total size: " << total_size << "; ";
        cout << "Cluster pos: ";
        for (int i = 0; i < cluster_pos.size(); i++)
        {
            cout << cluster_pos[i] << ", ";
        }
        cout << endl;
        cout << contents.size() << endl;
        for (int i = 0; i < contents.size(); i++)
        {
            contents[i]->displayInfo();
        }
    }


    void addNewFile_Directory(FileSystemEntity* f)
    {
        contents.push_back(f);
    }

    void read_next_sector(Drive* dr, wstring drivePath)
    {
        FileSystemEntity::read_next_sector(dr, drivePath);
    }

    void readData(Drive* dr, wstring drivePath)
    {
        readDirectoryData(dr, drivePath);
        for (int i = 0; i < contents.size(); i++)
        {
            if (dynamic_cast<Directory*>(contents[i]))
            {
                static_cast<Directory*>(contents[i])->readData(dr, drivePath);
            }
        }
    }
    void readDirectoryData(Drive* dr, wstring drivePath);

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
    vector<FileSystemEntity*> rootDirectories_Files;
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
    FAT32_BOOTSECTOR getBootSectorIn4() { return fat32bs; }

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
    void addNewFile_Directory(FileSystemEntity* f)
    {
        rootDirectories_Files.push_back(f);
    }
    void DisplayInfo()
    {
        cout << "Drive " << name << "\n";
        for (int i = 0; i < rootDirectories_Files.size(); i++)
            rootDirectories_Files[i]->displayInfo();
    }
    void readData(wstring drivePath)
    {
        for (int i = 0; i < rootDirectories_Files.size(); i++)
        {
            if (dynamic_cast<Directory*>(rootDirectories_Files[i]))
            {
                static_cast<Directory*>(rootDirectories_Files[i])->readData(this, drivePath);
            }
        }
    }
    ~Drive()
    {
        for (int i = 0; i < rootDirectories_Files.size(); i++)
            delete rootDirectories_Files[i];
    }
};

class Computer {
private:
    vector<Drive*> root_Drives;
public:
    void addRootDrive(Drive*& d) {
        root_Drives.push_back(d);
    }
    void read_FAT32_Drives();
    void read_FAT32_BootSector(int ith_drive, wstring drivePath);
    void read_FAT32_RDET(int ith_drive, wstring drivePath);
    void detectFormat();
    void GetRemovableDriveNames();
    void DispayInfo()
    {
        for (int i = 0; i < root_Drives.size(); i++)
        {
            root_Drives[i]->DisplayInfo();
        }
    }
    void readData(wstring drivePath)
    {
        for (int i = 0; i < root_Drives.size(); i++)
        {
            if (root_Drives[i]->getType() == "FAT32")
            {
                root_Drives[i]->readData(drivePath);
            }
        }
    }
};


int littleEndianByteArrayToInt(const BYTE* byteArray, size_t length);
wstring stringToWideString(const string& str);
string createName(vector <vector <BYTE>> extra_entry, vector <BYTE> main_entry);
Date createDate(vector <BYTE> main_entry);
Time createTime(vector <BYTE> main_entry);

#endif