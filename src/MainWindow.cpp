//
// Created by benichn on 14/08/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "MainWindow.h"

#include <magic_enum.hpp>

#include "app.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initScanner().then([=] { searchDevices(); });
}

MainWindow::~MainWindow()
{
    delete ui;
}

Task<> MainWindow::initScanner()
{
    auto& scanner = app().scanner();
    if (scanner.initialized()) co_return;
    ui->statusbar->showMessage("Initialisation SANE ...");
    const auto sta = co_await scanner.init();
    ostringstream msg;
    if (sta)
    {
        msg << "SANE initialisé";
    }
    else
    {
        msg << "Erreur pendant l'initialisation de SANE : ";
        msg << magic_enum::enum_name(sta.status());
    }
    ui->statusbar->showMessage(msg.str().c_str());
}

Task<> MainWindow::searchDevices()
{
    auto& scanner = app().scanner();
    ui->statusbar->showMessage("Recherche d'appareils de scan ...");
    const auto sta = co_await scanner.updateDevices();
    ostringstream msg;
    if (sta)
    {
        msg << scanner.devices().size();
        msg << " appareils de scan toruvés";
        co_await scanner.setCurrentDevice(app().appSettings().scanDevName());
    }
    else
    {
        msg << "Erreur pendant la recherche d'appareils de scan : ";
        msg << magic_enum::enum_name(sta.status());
    }
    ui->statusbar->showMessage(msg.str().c_str());
}
