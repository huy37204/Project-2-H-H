#include "Header.h"

int byteToTwosComplement(int byteValue) {
    if (byteValue & 0x80) { 
        return byteValue - 256;
    }
    else {
        return byteValue; 
    }
}



void Computer::read_NTFS_VBR(int ith_drive, wstring drivePath)
{
    HANDLE hDrive = CreateFile(drivePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) {
        cout << "Failed to open physical drive." << endl;
        return;
    }
    SetFilePointer(hDrive, 0, NULL, FILE_BEGIN);
    DWORD bytesRead;
    BYTE vbr[512];
    if (!ReadFile(hDrive, vbr, sizeof(vbr), &bytesRead, NULL)) {
        wcerr << "Failed to read boot sector from physical drive." << endl;
        CloseHandle(hDrive);
        return;
    }
    int byte_per_sector = vbr[0x0B] | (vbr[0x0B + 1] << 8);
    int sector_per_cluster = vbr[0x0D];
    int sum_sector_of_drive = vbr[0x28] | (vbr[0x28 + 1] << 8) | (vbr[0x28 + 2] << 16) | (vbr[0x28 + 3] << 24) | (vbr[0x28 + 4] << 32) | (vbr[0x28 + 5] << 40) | (vbr[0x28 + 6] << 48) | (vbr[0x28 + 7] << 56);
    int started_cluster_of_MFT = vbr[0x30] | (vbr[0x30 + 1] << 8) | (vbr[0x30 + 2] << 16) | (vbr[0x30 + 3] << 24) | (vbr[0x30 + 4] << 32) | (vbr[0x30 + 5] << 40) | (vbr[0x30 + 6] << 48) | (vbr[0x30 + 7] << 56);
    int started_cluster_of_extra_MFT = vbr[0x38] | (vbr[0x38 + 1] << 8) | (vbr[0x38 + 2] << 16) | (vbr[0x38 + 3] << 24) | (vbr[0x38 + 4] << 32) | (vbr[0x38 + 5] << 40) | (vbr[0x38 + 6] << 48) | (vbr[0x38 + 7] << 56);
    int byte_per_MFT_entry = pow(2, abs(byteToTwosComplement(vbr[0x40])));
    cout << "Byte per sector: " << byte_per_sector << endl;
    cout << "Sector per cluster: " << sector_per_cluster << endl;
    cout << "Total sectors of drive: " << sum_sector_of_drive << endl;
    cout << "Started cluster of MFT: " << started_cluster_of_MFT << endl;
    cout << "Started cluster of extra MFT: " << started_cluster_of_extra_MFT << endl;
    cout << "Byte per MFT entry: " << byte_per_MFT_entry << endl;
    root_Drives[ith_drive]->set_ntfs_vbr(byte_per_sector, sector_per_cluster, sum_sector_of_drive, started_cluster_of_MFT, started_cluster_of_extra_MFT, byte_per_MFT_entry);
}

void Computer::read_NTFS_MFT(int ith_drive, wstring drivePath) {
    NTFS_VBR vbr = root_Drives[ith_drive]->getVBRIn4();
    long long main_mft_offset_byte = (long long)vbr.byte_per_sector * vbr.sector_per_cluster * vbr.started_cluster_of_MFT;
    HANDLE hDrive = CreateFile(drivePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDrive == INVALID_HANDLE_VALUE) {
        cout << "Failed to open physical drive." << endl;
        return;
    }

    DWORD lowOffset = static_cast<DWORD>(main_mft_offset_byte & 0xFFFFFFFF);
    DWORD highOffset = static_cast<DWORD>((main_mft_offset_byte >> 32) & 0xFFFFFFFF);
    LARGE_INTEGER li;
    DWORD result = SetFilePointer(hDrive, lowOffset, reinterpret_cast<PLONG>(&highOffset), FILE_BEGIN);
    if (result == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
        cerr << "Failed to set file pointer." << endl;
        CloseHandle(hDrive);
        return;
    }
    DWORD bytesRead;
    BYTE* mft = new BYTE[vbr.byte_per_MFT_entry];
    if (!ReadFile(hDrive, mft, vbr.byte_per_MFT_entry, &bytesRead, NULL)) {
        wcerr << "Failed to read cluster from physical drive." << endl;
        CloseHandle(hDrive);
        delete[] mft;
        return;
    }
    if (mft[0x00] == 70 && (mft[0x16] == 0x01 || mft[0x16] == 0x03)) // FILE || BAAD, 0x01: File dang dung, 0x03: Thu muc dang dung
    {
        Header_MFT_Entry mft_header;
        mft_header.started_attribute_offset = mft[0x14] | mft[0x15] << 8;
        mft_header.flag = mft[0x16] | mft[0x17] << 8;
        mft_header.byte_used = mft[0x18] | (mft[0x19] << 8) | (mft[0x20] << 16) | (mft[0x21] << 24);
        mft_header.byte_of_MFT_entry = mft[0x1C] | (mft[0x1D] << 8) | (mft[0x1E] << 16) | (mft[0x1F] << 24);
        mft_header.ID = mft[0x2C] | (mft[0x2D] << 8) | (mft[0x2E] << 16) | (mft[0x2F] << 21);
        cout << "Started Attribute Offset: " << mft_header.started_attribute_offset << endl;
        cout << "Flag: " << mft_header.flag << endl;
        cout << "Byte Used: " << mft_header.byte_used << endl;
        cout << "Byte of MFT Entry: " << mft_header.byte_of_MFT_entry << endl;
        cout << "ID: " << mft_header.ID << endl;
        root_Drives[ith_drive]->setMFT(mft_header);

        //Doc attributes
        int size = mft[0x04] | (mft[0x05] << 8) | (mft[0x06] << 16) | (mft[0x07] << 24);
        int started_byte = mft_header.started_attribute_offset;
        int cnt = 1;
        while (started_byte < vbr.byte_per_MFT_entry && cnt <= 2)
        {
            BYTE* h_attr = new BYTE[size];
            copy(mft + started_byte, mft + started_byte + size, h_attr);
            Header_Attribute h;
            h.type_id = h_attr[0x00] | (h_attr[0x01] << 8) | (h_attr[0x02] << 16) | (h_attr[0x03] << 24);
            h.size_of_attribute = h_attr[0x04] | (h_attr[0x05] << 8) | (h_attr[0x06] << 16) | (h_attr[0x07] << 24);
            h.flag_resident = h_attr[0x08];
            h.length_name_attribute = h_attr[9];
            h.offset_of_name = h_attr[10] | (h_attr[11] << 8);
            h.flags = h_attr[12] | (h_attr[13] << 8);
            h.attribute_id = h_attr[14] | (h_attr[15] << 8);
            cout << "Type ID: " << h.type_id << endl;
            cout << "Size of Attribute: " << h.size_of_attribute << endl;
            cout << "Flag Resident: " << h.flag_resident << endl;
            cout << "Length Name Attribute: " << h.length_name_attribute << endl;
            cout << "Offset of Name: " << h.offset_of_name << endl;
            cout << "Flags: " << h.flags << endl;
            cout << "Attribute ID: " << h.attribute_id << endl;
            root_Drives[ith_drive]->pushHeaderAttribute(h);
            cnt++;
            started_byte += h.size_of_attribute;
        }
        
    }
    delete[] mft;
    CloseHandle(hDrive);
}