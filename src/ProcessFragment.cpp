//
// Created by benichn on 28/07/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "ProcessFragment.h"
#include "ui_ProcessFragment.h"

#include "app.h"
#include "utils.h"
#include "components/ScanWidget.h"

ProcessFragment::ProcessFragment(QWidget* parent) :
    QWidget(parent), ui(new Ui::ProcessFragment)
{
    ui->setupUi(this);
    connect(ui->pageNav->list()->selectionModel(), &QItemSelectionModel::selectionChanged, [=]
    {
        const int newId = uniqueSelectedId(ui->pageNav->list()->selectionModel());
        ui->previewer->setPageId(newId);
        ui->timeline->setPageId(newId);
        ui->propsEditor->setSource(newId == -1 ? nullpropsource : PropsSource(in_place_index<PTY_PAGE>, newId));
    });
    connect(ui->actionD_marrer_la_selection, &QAction::triggered, [=]
    {
        const auto& ids = app().book().ids();
        auto& works = app().works();
        for (const auto& idx : ui->pageNav->list()->selectionModel()->selectedIndexes())
        {
            works.enqueue({ids.at(idx.row()), true, false});
        }
    });
    connect(ui->actionD_marrer_une_tape_sur_la_s_lection, &QAction::triggered, [=]
    {
        const auto& ids = app().book().ids();
        auto& works = app().works();
        for (const auto& idx : ui->pageNav->list()->selectionModel()->selectedIndexes())
        {
            works.enqueue({ids.at(idx.row()), false, false});
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
    connect(ui->actionSupprimer_la_s_lection, &QAction::triggered, [=]
    {
        auto& book = app().book();
        const auto& ids = book.ids();
        const auto sids = ui->pageNav->list()->selectionModel()->selectedIndexes() | stv::transform([&](const auto& idx)
        {
            return ids.at(idx.row());
        }) | str::to<vector<int>>();
        book.removePages(sids);
    });
}

ProcessFragment::~ProcessFragment()
{
    delete ui;
}
