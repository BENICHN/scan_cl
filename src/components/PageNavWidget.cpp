//
// Created by benichn on 28/07/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_PageNavWidget.h" resolved

#include "PageNavWidget.h"

#include "ui_PageNavWidget.h"
#include "../data/book.h"
#include "models/PagesDelegate.h"
#include "models/PagesModel.h"


PageNavWidget::PageNavWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::PageNavWidget) {
    ui->setupUi(this);
    ui->list->setModel(new PagesModel(this));
    ui->list->setItemDelegate(new PagesDelegate(this));
}

PageNavWidget::~PageNavWidget() {
    delete ui;
}