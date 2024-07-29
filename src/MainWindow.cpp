//
// Created by benichn on 28/07/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "MainWindow.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget* parent) :
    QWidget(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pageNav->list()->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::pageNavSelectionChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::pageNavSelectionChanged()
{
    QModelIndexList idxs = ui->pageNav->list()->selectionModel()->selectedIndexes();
    ui->previewer->setPageId(idxs.size() == 1 ? idxs[0].row() : -1);
}
