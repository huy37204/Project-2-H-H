#ifndef QT_GUI_H
#define QT_GUI_H
#pragma once

#include "ui_Qt_GUI.h"
#include <QtWidgets/QMainWindow>
#include <QFileSystemModel> 

class Qt_GUI : public QMainWindow
{
    Q_OBJECT

public:
    Qt_GUI(QWidget *parent = nullptr);
    ~Qt_GUI();
    Ui::Qt_GUIClass* getUi() { return &ui; }
private:
    Ui::Qt_GUIClass ui;
    QFileSystemModel* model;
};
#endif