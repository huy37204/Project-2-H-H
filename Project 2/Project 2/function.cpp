#include "Header.h"

wstring stringToWideString(const string& str) {
    wstring wideStr(str.begin(), str.end());
    return wideStr;
}

int littleEndianByteArrayToInt(const BYTE* byteArray, size_t length) {
    int result = 0;
    for (size_t i = 0; i < length; ++i) {
        result |= (static_cast<int>(byteArray[i]) << (i * 8));
    }
    return result;
}
void read_FAT32_BootSector(Computer& MyPC, int ith_drive, wstring drivePath)
{
    HANDLE hDrive = CreateFile(drivePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) {
        cout << "Failed to open physical drive." << endl;
        return;
    }
    SetFilePointer(hDrive, 0, NULL, FILE_BEGIN);
    DWORD bytesRead;
    BYTE bootsector[512];
    if (!ReadFile(hDrive, bootsector, sizeof(bootsector), &bytesRead, NULL)) {
        wcerr << "Failed to read boot sector from physical drive." << endl;
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
    MyPC.root_Drives[ith_drive]->set_fat32_bootsector(byte_per_sector, sector_per_cluster, sector_before_FAT_table, num_of_FAT_tables, Volume_size, sector_per_FAT, first_cluster_of_RDET);
    cout << "Byte per sector: " << byte_per_sector << endl;
    cout << "Sector per cluster: " << sector_per_cluster << endl;
    cout << "Sector before FAT table: " << sector_before_FAT_table << endl;
    cout << "Num of FAT tables: " << num_of_FAT_tables << endl;
    cout << "Volume size: " << Volume_size << endl;
    cout << "Sector per FAT: " << sector_per_FAT << endl;
    cout << "First cluster of RDET: " << first_cluster_of_RDET << endl;
    wcout << endl;
    CloseHandle(hDrive);
}

string createName(vector <vector <BYTE>> extra_entry, vector <BYTE> main_entry)
{
    ostringstream oss;
    if (!extra_entry.empty())
    {
        for (int i = extra_entry.size() - 1; i >= 0; i--)
        {
            for (int j = 0x01; j < 0x01 + 10; j++)
            {
                if (extra_entry[i][j] == 0x0f)
                    return oss.str();
                oss << hex << extra_entry[i][j];
            }
            for (int j = 0x0E; j < 0x0E + 12; j++)
            {
                if (extra_entry[i][j] == 0x0f)
                    return oss.str();
                oss << hex << extra_entry[i][j];
            }
            for (int j = 0x1C; j < 0x1C + 4; j++)
            {
                if (extra_entry[i][j] == 0x0f)
                    return oss.str();
                oss << hex << extra_entry[i][j];
            }
        }
    }
    else
    {
        for (int i = 0x00; i <= 0x00 + 11; i++)
        {
            if (main_entry[i] == 0x0f)
                return oss.str();
            oss << hex << main_entry[i];
        }
    }
    return oss.str();
}

void read_FAT32_RDET(Computer& MyPC, int ith_drive, wstring drivePath)
{
    HANDLE hDrive = CreateFile(drivePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) {
        cout << "Failed to open physical drive." << endl;
        return;
    }
    DWORD bytesRead;
    int bytes_read = 0;
    bool is_end = 0;
    while (!is_end)
    {
        SetFilePointer(hDrive, MyPC.root_Drives[ith_drive]->getStartedByteRDET() + bytes_read, NULL, FILE_BEGIN);
        bytes_read += 512;
        BYTE rdet[512];
        if (!ReadFile(hDrive, rdet, sizeof(rdet), &bytesRead, NULL)) {
            wcerr << "Failed to read boot sector from physical drive." << endl;
            CloseHandle(hDrive);
            return;
        }
        int start_byte = 0; // Cho vong lap chay 32 byte cho moi vong lap
        vector <vector <BYTE>> extra_entry;
        vector <BYTE> main_entry;
        while (start_byte < 512)
        {
            int attribute = rdet[start_byte + 0x0B]; //0-0-A-D-V-S-H-R
            if (attribute == 0x0F)
            {
                vector<BYTE> temp_vec;
                copy(&rdet[start_byte], &rdet[start_byte + 32], back_inserter(temp_vec)); // Doc 32 byte vao entry phu
                extra_entry.push_back(temp_vec);
            }
            else if ((attribute == 0x10 || attribute == 0x20) && rdet[start_byte] != 0xE5)
            {
                copy(&rdet[start_byte], &rdet[start_byte + 32], back_inserter(main_entry)); // Doc 32 byte vao entry chinh
                string name = createName(extra_entry, main_entry);
                cout << name << endl;
                MyPC.root_Drives[ith_drive]->setName(name);
                extra_entry = vector <vector <BYTE>>{};
                main_entry = vector <BYTE>{};
            }
            else if ((attribute == 0x10 || attribute == 0x20) && rdet[start_byte] == 0xE5)
            {
                extra_entry = vector <vector <BYTE>>{};
            }
            else if (attribute == 0x00)
                is_end = 1;
            start_byte += 32;
        }
    }
    CloseHandle(hDrive);
   /* for (int i = 0; i < bytesRead; ++i) {
        wcout << hex << setw(2) << setfill(L'0') << (int)rdet[i] << L" ";
        if ((i + 1) % 16 == 0) {
            wcout << endl;
        }
    }*/

    /*HANDLE hDrive = CreateFile(drivePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) {
        cout << "Failed to open physical drive." << endl;
        return;
    }
    SetFilePointer(hDrive, 0, NULL, FILE_BEGIN);
    DWORD bytesRead;
    BYTE bootsector[512];
    if (!ReadFile(hDrive, bootsector, sizeof(bootsector), &bytesRead, NULL)) {
        wcerr << "Failed to read boot sector from physical drive." << endl;
        CloseHandle(hDrive);
        return;
    }*/
}

void read_FAT32_Drives(Computer& MyPC)
{
    int num_of_drives = MyPC.root_Drives.size();
    wstring wideDriveLetter = stringToWideString(MyPC.root_Drives[1]->getName());
    wstring drivePath = L"\\\\.\\" + wideDriveLetter;
    read_FAT32_BootSector(MyPC, 1,drivePath);
    read_FAT32_RDET(MyPC, 1, drivePath);
}

void detectFormat(Computer& MyPC)
{
    GetRemovableDriveNames(MyPC);
    string name;
    HANDLE hDrive = CreateFile(L"\\\\.\\PhysicalDrive1", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) {
        cout << "Failed to open physical drive." << std::endl;
        return;
    }
    BYTE mbr[512]; // MBR 512 bytes
    DWORD bytesRead;
    if (!ReadFile(hDrive, mbr, sizeof(mbr), &bytesRead, NULL)) {
        cout << "Failed to read MBR from physical drive." << std::endl;
        CloseHandle(hDrive);
        return;
    }
    int drive_order = 0;
    for (int i = 0x01BE + 4; i < sizeof(mbr); i += 16) {// offset 0x04 bat dau tu 0x01BE, Bang mo ta 1 partition cua MBR = 16 bytes 
        if (mbr[i] == 0x07) {
            MyPC.root_Drives[drive_order]->setType("NTFS");
        }
        else if (mbr[i] == 0x0C || mbr[i] == 0x0B) {
            MyPC.root_Drives[drive_order]->setType("FAT32");
        }
        else if (mbr[i] == 0x00)
            break;
        else {
            MyPC.root_Drives[drive_order]->setType("Unknown");
        }
        //Doc vi tri sector bat dau
        BYTE startSector[5];
        memcpy(startSector, &mbr[i + 4], 4);
        startSector[4] = '\0';
        MyPC.root_Drives[drive_order++]->setStartedByte(littleEndianByteArrayToInt(startSector, 4));
    }
    CloseHandle(hDrive);
}

void GetRemovableDriveNames(Computer& MyPC) {
    vector<string> driveNames;
    char LogicalDrives[MAX_PATH] = { 0 };
    DWORD dwResult = GetLogicalDriveStringsA(MAX_PATH, LogicalDrives);
    if (dwResult > 0 && dwResult <= MAX_PATH) {
        char* pszDrive = LogicalDrives;
        while (*pszDrive) {
            UINT driveType = GetDriveTypeA(pszDrive);
            if (driveType == DRIVE_REMOVABLE) {
                driveNames.push_back(pszDrive);
            }
            pszDrive += strlen(pszDrive) + 1;
        }
    }
    for (int i = 0; i < driveNames.size(); i++)
    {
        Drive* d = new Drive;
        d->setName(driveNames[i].substr(0, driveNames[i].size() - 1));
        MyPC.addRootDrive(d);
    }
}

