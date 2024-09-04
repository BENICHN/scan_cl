//
// Created by benichn on 13/08/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_PropsEditor.h" resolved

#include "PropsEditor.h"
#include "ui_PropsEditor.h"
#include "../../data/PageSettingsModel.h"
#include "../../data/StaticJsonDelegate.h"


void PropsEditor::setSource(const PropsSource& source)
{
    _source = source;
    updateSource();
}

PropsEditor::PropsEditor(QWidget* parent) :
    QWidget(parent), ui(new Ui::PropsEditor)
{
    ui->setupUi(this);
    ui->treeView->setModel(new PageSettingsModel(this));
    ui->treeView->setItemDelegate(new StaticJsonDelegate(this));
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    connect(qobject_cast<StaticJsonModel*>(ui->treeView->model()), &StaticJsonModel::jsonReset, [=]
    {
        ui->treeView->expandAll();
    });
}

PropsEditor::~PropsEditor()
{
    delete ui;
}

void PropsEditor::updateSource()
{
    const auto model = qobject_cast<PageSettingsModel*>(ui->treeView->model());
    model->setSource(_source);
}
