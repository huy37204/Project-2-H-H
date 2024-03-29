#include "Header.h"


int main() {
    Computer MyPC;
    detectFormat(MyPC);
    for (int i = 0; i < MyPC.root_Drives.size(); i++)
        cout << MyPC.root_Drives[i]->getName() << " " << MyPC.root_Drives[i]->getType() << endl;
    return 0;
}