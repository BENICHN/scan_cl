//
// Created by benichn on 04/09/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_BookEditor.h" resolved

#include "BookEditor.h"
#include "ui_BookEditor.h"
#include "../app.h"
#include "../data/StaticJsonDelegate.h"
#include "models/BookEditorModel.h"


BookEditor::BookEditor(QWidget* parent) :
    QWidget(parent), ui(new Ui::BookEditor)
{
    ui->setupUi(this);
    ui->treeView->setModel(new BookEditorModel(this));
    ui->treeView->setItemDelegate(new StaticJsonDelegate(this));
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeView->expandAll();
    auto& book = app().book();
    connect(&book, &Book::bookReset, [=, &book]
    {
        ui->titleEdit->setPlainText(book.title().c_str());
        ui->romanLimitEdit->setValue(book.romanLimit());
    });
    connect(ui->titleEdit, &QPlainTextEdit::textChanged, [=, &book] // ! plutot on quit ?
    {
        book.setTitle(ui->titleEdit->toPlainText().toStdString());
    });
    connect(ui->romanLimitEdit, &QSpinBox::valueChanged, [=, &book] // ! plutot on quit ?
    {
        book.setRomanLimit(ui->romanLimitEdit->value());
    });
    connect(qobject_cast<StaticJsonModel*>(ui->treeView->model()), &StaticJsonModel::jsonReset, [=]
    {
        ui->treeView->expandAll();
    });
}

BookEditor::~BookEditor()
{
    delete ui;
}
