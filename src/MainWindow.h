//
// Created by benichn on 14/08/24.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "imports/qcoroimports.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Task<> initScanner();
    Task<> searchDevices();
    Ui::MainWindow *ui;
};


#endif //MAINWINDOW_H
