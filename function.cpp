#include "Header.h"

void getInformation(Computer& MyPC)
{
    GetRemovableDriveNames(MyPC);
}

void detectFormat(Computer& MyPC)
{
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
            MyPC.root_Drives[drive_order++]->setType("NTFS");
        }
        else if (mbr[i] == 0x0C) {
            MyPC.root_Drives[drive_order++]->setType("FAT32");
        }
        else if (mbr[i] == 0x00)
            break;
        else {
            MyPC.root_Drives[drive_order++]->setType("Unknown");
        }
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
        d->setName(driveNames[i]);
        MyPC.addRootDrive(d);
    }
}