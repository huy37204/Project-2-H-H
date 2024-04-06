#include "Header.h"

int main(int argc, char* argv[])
{
    Computer MyPC;
    MyPC.detectFormat();
    MyPC.readDrives();
    QApplication a(argc, argv);
    QStandardItemModel model;
    Qt_GUI w(NULL);
    MyPC.DisplayGUI(model, w); 
    w.show();
    return a.exec();
}