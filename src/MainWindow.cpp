//
// Created by benichn on 28/07/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "app.h"
#include "components/ScanWidget.h"

MainWindow::MainWindow(QWidget* parent) :
    QWidget(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pageNav->list()->selectionModel(), &QItemSelectionModel::selectionChanged, [=]
    {
        const int newId = uniqueSelectedId();
        ui->previewer->setPageId(newId);
        ui->timeline->setPageId(newId);
        ui->propsEditor->setSource(PropsSource(in_place_index<PTY_PAGE>, newId));
    });
    connect(ui->actionD_marrer_la_selection, &QAction::triggered, [=]
    {
        const auto& ids = app().book().ids();
        auto& works = app().works();
        for (const auto& idx : ui->pageNav->list()->selectionModel()->selectedIndexes())
        {
            works.enqueue({ ids.at(idx.row()), true, false });
        }
    });
    connect(ui->actionD_marrer_une_tape_sur_la_s_lection, &QAction::triggered, [=]
    {
        const auto& ids = app().book().ids();
        auto& works = app().works();
        for (const auto& idx : ui->pageNav->list()->selectionModel()->selectedIndexes())
        {
            works.enqueue({ ids.at(idx.row()), false, false });
        }
    });
    connect(ui->actionR_initialier_la_s_lection, &QAction::triggered, [=]
    {
        auto& book = app().book();
        const auto& ids = book.ids();
        for (const auto& idx : ui->pageNav->list()->selectionModel()->selectedIndexes())
        {
            book.resetPage(ids.at(idx.row()));
        }
    });
    connect(ui->actionAcquisition, &QAction::triggered, [=]
    {
        (new ScanWidget)->showMaximized();
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
