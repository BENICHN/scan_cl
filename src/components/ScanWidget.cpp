//
// Created by benichn on 14/08/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ScanWidget.h" resolved

#include "ScanWidget.h"
#include "ui_ScanWidget.h"


ScanWidget::ScanWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::ScanWidget) {
    ui->setupUi(this);
}

ScanWidget::~ScanWidget() {
    delete ui;
}
