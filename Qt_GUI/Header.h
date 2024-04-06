#ifndef FILESYSTEMENTITY_H
#define FILESYSTEMENTITY_H
#pragma once
#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <QStandardItemModel>
#include <QtWidgets/QApplication>
#include <QDir>

#include "Qt_GUI.h"




class Drive;

struct Date
{
    int day, month, year;
};
struct Time
{
    int hour, minute, second, milisecond = 0;
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

struct NTFS_VBR
{
    int byte_per_sector;
    int sector_per_cluster;
    int sum_sector_of_drive;
    int started_cluster_of_MFT;
    int started_cluster_of_extra_MFT;
    int byte_per_MFT_entry;
};

struct Header_MFT_Entry
{
    int started_attribute_offset;
    int flag;
    int byte_used;
    int byte_of_MFT_entry;
    int ID;
};

struct Header_Attribute
{
    long long type_id; // Ma loai
    int size_of_attribute;
    int flag_non_resident;
    int length_name_attribute;
    int offset_of_name;
    int flags;
    int attribute_id;
    int attribute_data_size;
    int attribute_data_offset;
};


struct NTFS_MFT
{
    Header_MFT_Entry MFT_header;
    std::vector <Header_Attribute> header_attributes;
};

struct NonResidentInfo
{
    long long offset;
    long long datasize;
};
class FileSystemEntity {
protected:
    std::string name;
    std::string attributes;
    Date date_created;
    Time time_created;
    std::vector <int> cluster_pos;
    long long total_size;
    std::vector <std::vector<BYTE>>data;

    NTFS_MFT mft;
    std::string text;
    NonResidentInfo nonresidentinfo;
public:
    FileSystemEntity() = default;

    void setName(std::string name) { this->name = name; }
    std::string getName() { return name; }


    void setTime(Time t) { this->time_created = t; }
    void setDate(Date d) { this->date_created = d; }

    void add_cluster_pos(int pos) { cluster_pos.push_back(pos); }
    int get_pos_cluster(int index) { return cluster_pos[index]; }

    void setTotalSize(long long size) { this->total_size = size; }
    long long getTotalSize() { return total_size; }

    void Push_Data(std::vector<BYTE>DATA) { data.push_back(DATA); }

    void FAT32_Get_Cluster_Pos(std::vector <int>& vec)
    {
        vec = std::vector<int>{};
        for (int i = 0; i < cluster_pos.size(); i++)
        {
            vec.push_back(cluster_pos[i]);
        }
    }

    virtual void NTFS_Set_MFT(Header_MFT_Entry mft_header)
    {
        this->mft.MFT_header = mft_header;
    }
    NTFS_MFT getMFT() { return mft; }
    int NTFS_Get_ID() { return mft.MFT_header.ID; }
    void pushHeaderAttribute(Header_Attribute h) { mft.header_attributes.push_back(h); }

    void FAT32_Read_Next_Sector(Drive* dr, std::wstring drivePath);

    virtual void displayInfo() {};
    virtual ~FileSystemEntity() {};
};


class File : public FileSystemEntity {
private:
public:
    File() = default;
    void displayInfo() override {
        std::cout << "File: " << getName() << "; ";
        std::cout << "Date created: " << date_created.day << "/" << date_created.month << "/" << date_created.year << "; ";
        std::cout << "Time created: " << time_created.hour << ":" << time_created.minute << ":" << time_created.second << ":" << time_created.milisecond << "; ";
        std::cout << "Total size: " << total_size << "; ";
        for (int i = 0; i < cluster_pos.size(); i++)
        {
            std::cout << cluster_pos[i] << "; ";
        }
        std::cout << std::endl;
        std::cout << text << std::endl;
    }
    void FAT32_Read_Next_Sector(Drive* dr, std::wstring drivePath)
    {
        FileSystemEntity::FAT32_Read_Next_Sector(dr, drivePath);
    }
    void FAT32_Read_Data(Drive* dr, std::wstring drivePath);
    void byteArrayToString();
    void NTFS_Set_MFT(Header_MFT_Entry mft_header)
    {
        FileSystemEntity::NTFS_Set_MFT(mft_header);
    }

    void NTFS_Read_Resident_Data(BYTE* attr, Header_Attribute h); // Su dung trong Computer::NTFS_Read_MFT()
    void NTFS_Read_Non_Resident_Info(BYTE* attr, NTFS_VBR vbr, int datarun_offset, std::wstring drivePath, long long datasize);
    void NTFS_Nonresident_Read_Data(std::wstring drivePath, NTFS_VBR vbr); // Point den cluster chua data o ngoai vung MFT va xu ly du lieu
    void NTFS_read_data(std::wstring drivePath, NTFS_VBR vbr) // Doc nhung du lieu chua duoc doc trong MFT (Nonresident)
    {
        for (int i = 0; i < mft.header_attributes.size(); i++)
        {

            if (mft.header_attributes[i].type_id == 128 && mft.header_attributes[i].flag_non_resident == 1)
            {
                NTFS_Nonresident_Read_Data(drivePath, vbr);
                byteArrayToString(); // bo du lieu vo string text
            }
        }
    }
};


class Directory : public FileSystemEntity {
private:
    std::vector<FileSystemEntity*> contents;
public:
    Directory() = default;


    void displayInfo() override {
        std::cout << "Directory: " << getName() << "; ";
        std::cout << "Date created: " << date_created.day << "/" << date_created.month << "/" << date_created.year << "; ";
        std::cout << "Time created: " << time_created.hour << ":" << time_created.minute << ":" << time_created.second << ":" << time_created.milisecond << "; ";
        std::cout << "Total size: " << total_size << "; ";
        for (int i = 0; i < cluster_pos.size(); i++)
        {
            std::cout << cluster_pos[i] << ", ";
        }
        std::cout << std::endl;
        std::cout << contents.size() << std::endl;
        for (int i = 0; i < contents.size(); i++)
        {
            contents[i]->displayInfo();
        }
    }
    void DisplayGUI(QStandardItem*& Drive)
    {
        for (int i = 0; i < contents.size(); i++)
        {
            Directory* dir = dynamic_cast<Directory*>(contents[i]);
            if (dir)
            {
                QString qname = QString::fromStdString(dir->getName());
                QStandardItem* f = new QStandardItem(qname);
                Drive->appendRow(f);
                dir->DisplayGUI(f);
            }
            else
            {
                File* file = dynamic_cast<File*>(contents[i]);
                if (file)
                {
                    QString qname = QString::fromStdString(file->getName());
                    QStandardItem* f = new QStandardItem(qname);
                    Drive->appendRow(f);
                }
            }
        }
    }

    void addNewFile_Directory(FileSystemEntity* f)
    {
        contents.push_back(f);
    }

    void FAT32_Read_Next_Sector(Drive* dr, std::wstring drivePath)
    {
        FileSystemEntity::FAT32_Read_Next_Sector(dr, drivePath);
    }

    void FAT32_Read_Data(Drive* dr, std::wstring drivePath)
    {
        FAT32_ReadDirectoryData(dr, drivePath);
        for (int i = 0; i < contents.size(); i++)
        {
            if (dynamic_cast<Directory*>(contents[i]))
            {
                static_cast<Directory*>(contents[i])->FAT32_Read_Data(dr, drivePath);
            }
            else
            {
                static_cast<File*>(contents[i])->FAT32_Read_Data(dr, drivePath);
            }
        }
    }
    void FAT32_ReadDirectoryData(Drive* dr, std::wstring drivePath);
    void NTFS_Set_MFT(Header_MFT_Entry mft_header)
    {
        FileSystemEntity::NTFS_Set_MFT(mft_header);
    }
    Directory* NTFS_Find_Parent_Directory(int parent_id);
    void NTFS_Read_Data(std::wstring drivePath, NTFS_VBR vbr)
    {
        for (int i = 0; i < contents.size(); i++)
        {
            if (dynamic_cast<File*>(contents[i]))
            {
                static_cast<File*>(contents[i])->NTFS_read_data(drivePath, vbr);
            }
        }
    }
    ~Directory()
    {
        for (int i = 0; i < contents.size(); i++)
            delete contents[i];
    }
};



class Drive {
private:
    std::string name;
    std::string type;
    int started_byte_bootsector;
    int started_byte_rdet;
    std::vector<FileSystemEntity*> rootDirectories_Files;
    FAT32_BOOTSECTOR fat32bs;

    NTFS_VBR vbr;
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
        this->started_byte_rdet = (sector_per_FAT * num_of_FAT_tables + sector_before_FAT_table) * byte_per_sector;
    }

    void set_ntfs_vbr(int byte_per_sector, int sector_per_cluster, int sum_sector_of_drive, int started_cluster_of_MFT, int started_cluster_of_extra_MFT, int byte_per_MFT_entry) {
        this->vbr.byte_per_sector = byte_per_sector;
        this->vbr.sector_per_cluster = sector_per_cluster;
        this->vbr.sum_sector_of_drive = sum_sector_of_drive;
        this->vbr.started_cluster_of_MFT = started_cluster_of_MFT;
        this->vbr.started_cluster_of_extra_MFT = started_cluster_of_extra_MFT;
        this->vbr.byte_per_MFT_entry = byte_per_MFT_entry;
    }

    NTFS_VBR getVBRIn4() { return vbr; }

    FAT32_BOOTSECTOR getBootSectorIn4() { return fat32bs; }

    void setName(std::string name)
    {
        this->name = name;
    }
    std::string getName() { return this->name; }
    void setType(std::string type)
    {
        this->type = type;
    }
    std::string getType() { return this->type; }
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
        std::cout << "Drive " << name << " " << type << std::endl;
        for (int i = 0; i < rootDirectories_Files.size(); i++)
        {
            Directory* dir = dynamic_cast<Directory*>(rootDirectories_Files[i]);
            if (dir)
            {
                dir->displayInfo();
            }
            else
            {
                File* file = dynamic_cast<File*>(rootDirectories_Files[i]);
                if (file)
                {
                    file->displayInfo();
                }
            }
        }
    }
    void DisplayGUI(QStandardItem*& Drive)
    {
        for (int i = 0; i < rootDirectories_Files.size(); i++)
        {
            Directory* dir = dynamic_cast<Directory*>(rootDirectories_Files[i]);
            if (dir)
            {
                QString qname = QString::fromStdString(dir->getName());
                QStandardItem* f = new QStandardItem(qname);
                Drive->appendRow(f);
                dir->DisplayGUI(f);
            }
            else
            {
                File* file = dynamic_cast<File*>(rootDirectories_Files[i]);
                if (file)
                {
                    QString qname = QString::fromStdString(file->getName());
                    QStandardItem* f = new QStandardItem(qname);
                    Drive->appendRow(f);
                }
            }
        }
    }
    void readData(std::wstring drivePath)
    {
        for (int i = 0; i < rootDirectories_Files.size(); i++)
        {
            if (dynamic_cast<Directory*>(rootDirectories_Files[i]))
            {
                static_cast<Directory*>(rootDirectories_Files[i])->FAT32_Read_Data(this, drivePath);
            }
            else
            {
                static_cast<File*>(rootDirectories_Files[i])->FAT32_Read_Data(this, drivePath);
                static_cast<File*>(rootDirectories_Files[i])->byteArrayToString();
            }
        }
    }
    void NTFS_Read_Data(std::wstring drivePath)
    {

        for (int i = 0; i < rootDirectories_Files.size(); i++)
        {
            if (dynamic_cast<Directory*>(rootDirectories_Files[i]))
            {
                static_cast<Directory*>(rootDirectories_Files[i])->NTFS_Read_Data(drivePath, vbr);
            }
            else
            {
                static_cast<File*>(rootDirectories_Files[i])->NTFS_read_data(drivePath, vbr);
            }
        }
    }
    Directory* NTFS_Find_Parent_Directory(int parent_id);
    ~Drive()
    {
        for (int i = 0; i < rootDirectories_Files.size(); i++)
            delete rootDirectories_Files[i];
    }
};

class Computer {
private:
    std::vector<Drive*> root_Drives;
public:
    void addRootDrive(Drive*& d) {
        root_Drives.push_back(d);
    }
    void readDrives();
    void FAT32_Read_BootSector(int ith_drive, std::wstring drivePath);
    void FAT32_Read_RDET(int ith_drive, std::wstring drivePath);

    void NTFS_Read_VBR(int ith_drive, std::wstring drivePath);
    void NTFS_Read_MFT(int ith_drive, std::wstring drivePath);

    void detectFormat();
    void GetRemovableDriveNames();
    void DisplayInfo()
    {
        for (int i = 0; i < root_Drives.size(); i++)
        {
            root_Drives[i]->DisplayInfo();
        }
    }
    void readData(std::wstring drivePath)
    {
        for (int i = 0; i < root_Drives.size(); i++)
        {

            if (root_Drives[i]->getType() == "FAT32")
            {
                root_Drives[i]->readData(drivePath);
            }
            else
            {
                root_Drives[i]->NTFS_Read_Data(drivePath);
            }
        }
    }
    void DisplayGUI(QStandardItemModel &model, Qt_GUI &w)
    {


        for (int i = 0; i < root_Drives.size(); i++)
        {
            QStandardItem* driveItem = new QStandardItem(root_Drives[i]->getName().c_str());
            driveItem->setIcon(QIcon("D:\\Hoc tap\\He Dieu Hanh\\Qt_GUI\\Qt_GUI\\images.png"));

            model.appendRow(driveItem);
            root_Drives[i]->DisplayGUI(driveItem);
        }
        w.getUi()->treeView->setModel(&model);
    }
};


int littleEndianByteArrayToInt(const BYTE* byteArray, size_t length);
std::wstring stringToWideString(const std::string& str);
std::string FAT32_Create_Name(std::vector <std::vector <BYTE>> extra_entry, std::vector <BYTE> main_entry);
std::string NTFS_Create_Name(BYTE* attr, Header_Attribute h);
Date FAT32_Create_Date(std::vector <BYTE> main_entry);
Time FAT32_Create_Time(std::vector <BYTE> main_entry);
void NTFS_Create_Date_Time(BYTE* attr, Header_Attribute h, Date& d, Time& t);
int byteToTwosComplement(int byteValue);
int NTFS_Read_BITMAP(BYTE* attr, Header_Attribute h);
#endif