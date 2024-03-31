#include "Header.h"


int main() {
    Computer MyPC;
    MyPC.detectFormat();
    MyPC.read_FAT32_Drives();
    MyPC.DispayInfo();
    return 0;
}
