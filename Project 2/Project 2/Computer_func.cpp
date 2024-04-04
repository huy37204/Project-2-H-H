#include "Header.h"

void Computer::readDrives()
{
    for (int i = 0; i < root_Drives.size(); i++)
    {
        std::wstring wideDriveLetter = stringToWideString(root_Drives[i]->getName());
        std::wstring drivePath = L"\\\\.\\" + wideDriveLetter;
        if (root_Drives[i]->getType() == "NTFS")
        {
            NTFS_Read_VBR(i, drivePath);
            NTFS_Read_MFT(i, drivePath);
        }
        else if (root_Drives[i]->getType() == "FAT32")
        {
            FAT32_Read_BootSector(i, drivePath);
            FAT32_Read_RDET(i, drivePath);
        }
        readData(drivePath);
    }
}

void Computer::detectFormat()
{
    GetRemovableDriveNames();
    HANDLE hDrive = CreateFile(L"\\\\.\\PhysicalDrive1", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) {
        std::cout << "Failed to open physical drive." << std::endl;
        return;
    }
    BYTE mbr[512]; // MBR 512 bytes
    DWORD bytesRead;
    if (!ReadFile(hDrive, mbr, sizeof(mbr), &bytesRead, NULL)) {
        std::cout << "Failed to read MBR from physical drive." << std::endl;
        CloseHandle(hDrive);
        return;
    }
    int drive_order = 0;
    for (int i = 0x01BE + 4; i < sizeof(mbr); i += 16) {// offset 0x04 bat dau tu 0x01BE, Bang mo ta 1 partition cua MBR = 16 bytes 
        if (mbr[i] == 0x07) {
            root_Drives[drive_order]->setType("NTFS");
        }
        else if (mbr[i] == 0x0C || mbr[i] == 0x0B) {
            root_Drives[drive_order]->setType("FAT32");
        }
        else if (mbr[i] == 0x00)
            break;
        else {
            root_Drives[drive_order]->setType("Unknown");
        }

        BYTE startSector[5];
        memcpy(startSector, &mbr[i + 4], 4);
        startSector[4] = '\0';
        root_Drives[drive_order++]->setStartedByte(littleEndianByteArrayToInt(startSector, 4));
    }
    CloseHandle(hDrive);
}

void Computer::GetRemovableDriveNames() {
    std::vector<std::string> driveNames;
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
        addRootDrive(d);
    }
}
