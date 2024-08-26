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
    updateModel();
}

PropsEditor::PropsEditor(QWidget* parent) :
    QWidget(parent), ui(new Ui::PropsEditor)
{
    ui->setupUi(this);
}

PropsEditor::~PropsEditor()
{
    delete ui;
}

void PropsEditor::updateModel()
{
    switch (_source.index())
    {
    case PTY_NONE:
        ui->treeView->setModel(nullptr);
        break;
    case PTY_PAGE:
        {
            const int id = get<PTY_PAGE>(_source);
            ui->treeView->setModel(new PageSettingsModel(id, this)); // ! delete model ?
            ui->treeView->setItemDelegate(new StaticJsonDelegate(this));
        }
        break;
    case PTY_NUMBER:
        break;
    }
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeView->expandAll();
}
