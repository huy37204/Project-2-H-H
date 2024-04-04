#include "Header.h"



void Computer::FAT32_Read_BootSector(int ith_drive, std::wstring drivePath)
{
    HANDLE hDrive = CreateFile(drivePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) {
        std::cout << "Failed to open physical drive." << std::endl;
        return;
    }
    SetFilePointer(hDrive, 0, NULL, FILE_BEGIN);
    DWORD bytesRead;
    BYTE bootsector[512];
    if (!ReadFile(hDrive, bootsector, sizeof(bootsector), &bytesRead, NULL)) {
        std::wcerr << "Failed to read boot sector from physical drive." << std::endl;
        CloseHandle(hDrive);
        return;
    }
    int byte_per_sector = bootsector[11] | (bootsector[12] << 8);
    int sector_per_cluster = bootsector[13];
    int sector_before_FAT_table = bootsector[14] | (bootsector[15] << 8);
    int num_of_FAT_tables = bootsector[16];
    int Volume_size = bootsector[32] | (bootsector[33] << 8) | (bootsector[34] << 16) | (bootsector[35] << 24);
    int sector_per_FAT = bootsector[36] | (bootsector[37] << 8) | (bootsector[38] << 16) | (bootsector[39] << 24);
    int first_cluster_of_RDET = bootsector[44] | (bootsector[45] << 8) | (bootsector[46] << 16) | (bootsector[47] << 24);
    root_Drives[ith_drive]->set_fat32_bootsector(byte_per_sector, sector_per_cluster, sector_before_FAT_table, num_of_FAT_tables, Volume_size, sector_per_FAT, first_cluster_of_RDET);
    std::cout << "Byte per sector: " << byte_per_sector << std::endl;
    std::cout << "Sector per cluster: " << sector_per_cluster << std::endl;
    std::cout << "Sector before FAT table: " << sector_before_FAT_table << std::endl;
    std::cout << "Num of FAT tables: " << num_of_FAT_tables << std::endl;
    std::cout << "Volume size: " << Volume_size << std::endl;
    std::cout << "Sector per FAT: " << sector_per_FAT << std::endl;
    std::cout << "First cluster of RDET: " << first_cluster_of_RDET << std::endl;
    std::wcout << std::endl;
    CloseHandle(hDrive);
}

std::string FAT32_Create_Name(std::vector <std::vector <BYTE>> extra_entry, std::vector <BYTE> main_entry)
{
    std::ostringstream oss;
    if (!extra_entry.empty())
    {
        for (int i = extra_entry.size() - 1; i >= 0; i--)
        {
            for (int j = 0x01; j < 0x01 + 10; j++)
            {
                if (extra_entry[i][j] == 0xFF)
                    return oss.str();
                oss << std::hex << extra_entry[i][j];
            }
            for (int j = 0x0E; j < 0x0E + 12; j++)
            {
                if (extra_entry[i][j] == 0xFF)
                    return oss.str();
                oss << std::hex << extra_entry[i][j];
            }
            for (int j = 0x1C; j < 0x1C + 4; j++)
            {
                if (extra_entry[i][j] == 0xFF)
                    return oss.str();
                oss << std::hex << extra_entry[i][j];
            }
        }
    }
    else
    {
        for (int i = 0x00; i <= 0x00 + 11; i++)
        {
            if (main_entry[i] == 0xFF)
                return oss.str();
            oss << std::hex << main_entry[i];
        }
    }
    return oss.str();
}
Date FAT32_Create_Date(std::vector <BYTE> main_entry)
{
    std::vector <BYTE> bigedian_date(2);
    for (int i = 0; i <= 1; i++)
    {
        bigedian_date[i] = main_entry[0x10 + 1 - i];
    }
    Date d;
    d.year =  1980 + ((bigedian_date[0] & 0xFE) >> 1);
    d.month = ((bigedian_date[0]) & 0x01) << 3 | ((bigedian_date[1] & 0xE0) >> 5);
    d.day = (bigedian_date[1]) & 0x1F;
    return d;
}
Time FAT32_Create_Time(std::vector <BYTE> main_entry)
{
    std::vector <BYTE> bigedian_time(3);
    for (int i = 0; i <= 2; i++)
    {
        bigedian_time[i] = main_entry[0x0D + 2 - i];
    }
    Time t;
    t.hour = (bigedian_time[0] >> 3) & 0x1F;
    t.minute = ((bigedian_time[0] & 0x07) << 3) | ((bigedian_time[1] >> 5) & 0x07);
    t.second = ((bigedian_time[1] & 0x1F) << 1 )| ((bigedian_time[2] >> 7) & 0x01);
    t.milisecond = (bigedian_time[2] & 0x7F);

    return t;
}

void Computer::FAT32_Read_RDET(int ith_drive, std::wstring drivePath)
{
    HANDLE hDrive = CreateFile(drivePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) {
        std::cout << "Failed to open physical drive." << std::endl;
        return;
    }
    DWORD bytesRead;
    int bytes_read = 0;
    bool is_end = 0;
    while (!is_end)
    {
        SetFilePointer(hDrive, root_Drives[ith_drive]->getStartedByteRDET() + bytes_read, NULL, FILE_BEGIN);
        bytes_read += 512;
        BYTE rdet[512];
        if (!ReadFile(hDrive, rdet, sizeof(rdet), &bytesRead, NULL)) {
            std::wcerr << "Failed to read boot sector from physical drive." << std::endl;
            CloseHandle(hDrive);
            return;
        }
        /*for (size_t i = 0; i < 512; ++i) {
            cout << setw(2) << setfill('0') << hex << static_cast<int>(rdet[i]) << " ";
            if ((i + 1) % 16 == 0) {
                cout << endl;
            }
        }*/
        int start_byte = 0; // Cho vong lap chay 32 byte cho moi vong lap
        std::vector <std::vector <BYTE>> extra_entry;
        std::vector <BYTE> main_entry;
        while (start_byte < 512)
        {
            int attribute = rdet[start_byte + 0x0B]; //0-0-A-D-V-S-H-R
            if (attribute == 0x0F)
            {
                std::vector<BYTE> temp_vec;
                std::copy(&rdet[start_byte], &rdet[start_byte + 32], back_inserter(temp_vec)); // Doc 32 byte vao entry phu
                extra_entry.push_back(temp_vec);
            }
            else if ((attribute == 0x10 || attribute == 0x20) && rdet[start_byte] != 0xE5)
            {
                copy(&rdet[start_byte], &rdet[start_byte + 32], back_inserter(main_entry)); // Doc 32 byte vao entry chinh
                std::string name = FAT32_Create_Name(extra_entry, main_entry);
                Date d = FAT32_Create_Date(main_entry);
                Time t = FAT32_Create_Time(main_entry);
                int started_cluster = main_entry[0x1A] | (main_entry[0x1A + 1] << 8);
                long long total_size = main_entry[0x1C] | (main_entry[0x1C + 1] << 8) | (main_entry[0x1C + 2] << 16) | (main_entry[0x1C + 3] << 24);
                if (attribute == 0x20)
                {
                    File* newFile = new File;
                    newFile->setName(name);
                    newFile->setDate(d);
                    newFile->setTime(t);
                    newFile->add_cluster_pos(started_cluster);
                    newFile->setTotalSize(total_size);
                    newFile->FAT32_Read_Next_Sector(root_Drives[ith_drive], drivePath);
                    root_Drives[ith_drive]->addNewFile_Directory(newFile);
                }
                else
                {
                    Directory* newDirectory = new Directory;
                    newDirectory->setName(name);
                    newDirectory->setDate(d);
                    newDirectory->setTime(t);
                    newDirectory->add_cluster_pos(started_cluster);
                    newDirectory->setTotalSize(total_size);
                    newDirectory->FAT32_Read_Next_Sector(root_Drives[ith_drive], drivePath);
                    root_Drives[ith_drive]->addNewFile_Directory(newDirectory);
                }
                extra_entry.clear();
                main_entry.clear();
            }
            else if ((attribute == 0x10 || attribute == 0x20) && rdet[start_byte] == 0xE5)
            {
                extra_entry.clear();
                main_entry.clear();
            }
            else if (attribute == 0x00)
            {
                is_end = 1;
            }
            start_byte += 32;
        }
    }
    CloseHandle(hDrive);
}

void FileSystemEntity::FAT32_Read_Next_Sector(Drive* dr, std::wstring drivePath)
{
    FAT32_BOOTSECTOR bs = dr->getBootSectorIn4();
    HANDLE hDrive = CreateFile(drivePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) {
        std::cout << "Failed to open physical drive." << std::endl;
        return;
    }
    DWORD bytesRead;
    int cluster_pos = get_pos_cluster(0) * 4;
    BYTE* fat1 = new BYTE[bs.byte_per_sector * bs.sector_per_FAT];
    while (true)
    {
        SetFilePointer(hDrive, bs.byte_per_sector * bs.sector_before_FAT_table, NULL, FILE_BEGIN);

        if (!ReadFile(hDrive, fat1, bs.byte_per_sector * bs.sector_per_FAT, &bytesRead, NULL)) {
            std::wcerr << "Failed to read FAT1 from physical drive." << std::endl;
            CloseHandle(hDrive);
            delete[] fat1;
            return;
        }
        bool is_empty = 1;
        for (int i = 0; i < 4; i++)
        {
            int value = fat1[cluster_pos + i];
            if (value != 0x00)
                is_empty = 0;
            if (!is_empty && value != 0xFF)
                break;
            if (value == 0xFF || value == 0xF7 || (value == 0x00 && is_empty))
                return;
        }
        cluster_pos = fat1[cluster_pos] | (fat1[cluster_pos + 1] << 8) | (fat1[cluster_pos + 2] << 16) | (fat1[cluster_pos + 3] << 24);
        add_cluster_pos(cluster_pos);
    }
    delete[] fat1;
}

void File::byteArrayToString()
{
    for (const auto& cluster : this->data) {
        for (BYTE byte : cluster) {
            this->text.push_back(static_cast<char>(byte));
        }
    }
}

void File::FAT32_Read_Data(Drive* dr, std::wstring drivePath)
{
    FAT32_BOOTSECTOR bs = dr->getBootSectorIn4();
    std::vector <int> pos_cluster;
    FAT32_Get_Cluster_Pos(pos_cluster);
    HANDLE hDrive = CreateFile(drivePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) {
        std::cout << "Failed to open physical drive." << std::endl;
        return;
    }
    DWORD bytesRead;
    int bytes_per_cluster = bs.byte_per_sector * bs.sector_per_cluster;
    for (int i = 0; i < pos_cluster.size(); i++)
    {
        int byte_cluster_pos = bs.byte_per_sector * (bs.sector_before_FAT_table + bs.num_of_FAT_tables * bs.sector_per_FAT + (pos_cluster[i] - bs.first_cluster_of_RDET) * bs.sector_per_cluster);
        SetFilePointer(hDrive, byte_cluster_pos, NULL, FILE_BEGIN);
        BYTE* data = new BYTE[bytes_per_cluster];
        if (!ReadFile(hDrive, data, bytes_per_cluster, &bytesRead, NULL)) {
            std::wcerr << "Failed to read boot sector from physical drive." << std::endl;
            CloseHandle(hDrive);
            return;
        }
        std::vector <BYTE> data_each_cluster; data_each_cluster.assign(data, data + bytes_per_cluster);
        Push_Data(data_each_cluster);
        delete[] data;
    }
    byteArrayToString();
    CloseHandle(hDrive);
}

void Directory::FAT32_ReadDirectoryData(Drive* dr, std::wstring drivePath)
{
    FAT32_BOOTSECTOR bs = dr->getBootSectorIn4();
    std::vector <int> pos_cluster;
    FAT32_Get_Cluster_Pos(pos_cluster);
    HANDLE hDrive = CreateFile(drivePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) {
        std::cout << "Failed to open physical drive." << std::endl;
        return;
    }
    DWORD bytesRead;
    int bytes_per_cluster = bs.byte_per_sector * bs.sector_per_cluster;
    for (int i = 0; i < pos_cluster.size(); i++)
    {
        int byte_cluster_pos = bs.byte_per_sector * (bs.sector_before_FAT_table + bs.num_of_FAT_tables * bs.sector_per_FAT + (pos_cluster[i] - bs.first_cluster_of_RDET) * bs.sector_per_cluster);
        SetFilePointer(hDrive, byte_cluster_pos, NULL, FILE_BEGIN);
        BYTE* data = new BYTE[bytes_per_cluster];
        if (!ReadFile(hDrive, data, bytes_per_cluster, &bytesRead, NULL)) {
            std::wcerr << "Failed to read boot sector from physical drive." << std::endl;
            CloseHandle(hDrive);
            return;
        }
        std::vector <BYTE> data_each_cluster; data_each_cluster.assign(data, data + bytes_per_cluster);
        Push_Data(data_each_cluster);
        int start_byte = 64; // Cho vong lap chay 32 byte cho moi vong lap
        std::vector <std::vector <BYTE>> extra_entry;
        std::vector <BYTE> main_entry;
        while (start_byte < bytes_per_cluster)
        {
            int attribute = data[start_byte + 0x0B]; //0-0-A-D-V-S-H-R
            if (attribute == 0x0F)
            {
                std::vector<BYTE> temp_vec;
                copy(&data[start_byte], &data[start_byte + 32], back_inserter(temp_vec)); // Doc 32 byte vao entry phu
                extra_entry.push_back(temp_vec);
            }
            else if ((attribute == 0x10 || attribute == 0x20) && data[start_byte] != 0xE5)
            {
                copy(&data[start_byte], &data[start_byte + 32], back_inserter(main_entry)); // Doc 32 byte vao entry chinh
                std::string name = FAT32_Create_Name(extra_entry, main_entry);
                Date d = FAT32_Create_Date(main_entry);
                Time t = FAT32_Create_Time(main_entry);
                int started_cluster = main_entry[0x1A] | (main_entry[0x1A + 1] << 8);
                long long total_size = main_entry[0x1C] | (main_entry[0x1C + 1] << 8) | (main_entry[0x1C + 2] << 16) | (main_entry[0x1C + 3] << 24);
                if (attribute == 0x20)
                {
                    File* newFile = new File;
                    newFile->setName(name);
                    newFile->setDate(d);
                    newFile->setTime(t);
                    newFile->add_cluster_pos(started_cluster);
                    newFile->setTotalSize(total_size);
                    newFile->FAT32_Read_Next_Sector(dr, drivePath);
                    this->addNewFile_Directory(newFile);
                }
                else
                {
                    Directory* newDirectory = new Directory;
                    newDirectory->setName(name);
                    newDirectory->setDate(d);
                    newDirectory->setTime(t);
                    newDirectory->add_cluster_pos(started_cluster);
                    newDirectory->setTotalSize(total_size);
                    newDirectory->FAT32_Read_Next_Sector(dr, drivePath);
                    this->addNewFile_Directory(newDirectory);
                }
                extra_entry = std::vector <std::vector <BYTE>>{};
                main_entry = std::vector <BYTE>{};
            }
            else if ((attribute == 0x10 || attribute == 0x20) && data[start_byte] == 0xE5)
            {
                extra_entry = std::vector <std::vector <BYTE>>{};
                main_entry = std::vector <BYTE>{};
            }
            start_byte += 32;
        }
        delete[] data;
    }
    CloseHandle(hDrive);
}




