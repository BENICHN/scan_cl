//
// Created by benichn on 28/07/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "app.h"

MainWindow::MainWindow(QWidget* parent) :
    QWidget(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pageNav->list()->selectionModel(), &QItemSelectionModel::selectionChanged, [=]
    {
        const int newId = uniqueSelectedId();
        ui->previewer->setPageId(newId);
        ui->timeline->setPageId(newId);
    });
    connect(ui->pushButton, &QAbstractButton::clicked, [=]
    {
        int id = uniqueSelectedId();
        if (id != -1) app().works().enqueue({id, true, false});
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::uniqueSelectedId() const
{
    auto idxs = ui->pageNav->list()->selectionModel()->selectedIndexes();
    return idxs.size() == 1 ? app().book().ids().at(idxs[0].row()) : -1;
}
