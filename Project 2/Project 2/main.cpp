#include "Header.h"


int main() {
    Computer MyPC;
    detectFormat(MyPC);
    for (int i = 0; i < MyPC.root_Drives.size(); i++)
    {
        cout << MyPC.root_Drives[i]->getName() << " " << MyPC.root_Drives[i]->getType() << endl;
        cout << MyPC.root_Drives[i]->getStartedByte() << endl;
    }
    read_FAT32_Drives(MyPC);
    MyPC.root_Drives[1]->DisplayInfo();
    return 0;
}
