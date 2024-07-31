//
// Created by benichn on 28/07/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "MainWindow.h"

#include "app.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent) :
    QWidget(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pageNav->list()->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::pageNavSelectionChanged);
    connect(ui->pushButton, &QAbstractButton::clicked, [this]
    {
        app()->works()->enqueue({ uniqueSelectedId(), true, false });
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::uniqueSelectedId() const
{
    QModelIndexList idxs = ui->pageNav->list()->selectionModel()->selectedIndexes();
    return idxs.size() == 1 ? app()->book()->id(idxs[0].row()) : -1;
}

void MainWindow::pageNavSelectionChanged()
{
    const int newId = uniqueSelectedId();
    ui->previewer->setPageId(newId);
    ui->timeline->setPageId(newId);
}
