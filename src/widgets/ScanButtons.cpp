//
// Created by benichn on 20/08/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ScanButtons.h" resolved

#include "ScanButtons.h"
#include "ui_ScanButtons.h"


ScanButtons::ScanButtons(QWidget* parent) :
    QWidget(parent), ui(new Ui::ScanButtons)
{
    ui->setupUi(this);
    connect(ui->b_del, &QAbstractButton::clicked, [=]
    {

    });
}

ScanButtons::~ScanButtons()
{
    delete ui;
}

ImageViewerWidget& ScanButtons::iv()
{
    return *ui->iv;
}

void ScanButtons::keyPressEvent(QKeyEvent* event)
{
    switch (_heldKey)
    {
    case Qt::Key_Escape:
        switch (event->key())
        {
        case Qt::Key_B:
            ui->b_b->setChecked(true);
            break;
        case Qt::Key_G:
            ui->b_g->setChecked(true);
            break;
        case Qt::Key_C:
            ui->b_c->setChecked(true);
            break;
        case Qt::Key_F:
            ui->b_f->toggle();
            break;
        case Qt::Key_H:
            _heldKey = Qt::Key_H;
            return;
        case Qt::Key_V:
            _heldKey = Qt::Key_V;
            return;
        case Qt::Key_M:
            ui->b_m->toggle();
            break;
        case Qt::Key_Delete:
            ui->b_del->click();
            break;
        }
        break;
    case Qt::Key_H:
        switch (event->key())
        {
    case Qt::Key_L:
        ui->b_hl->setChecked(true);
        break;
    case Qt::Key_C:
        ui->b_hc->setChecked(true);
        break;
    case Qt::Key_R:
        ui->b_hr->setChecked(true);
        break;
        }
        break;
    case Qt::Key_V:
        switch (event->key())
        {
    case Qt::Key_T:
        ui->b_vt->setChecked(true);
        break;
    case Qt::Key_H:
        ui->b_vh->setChecked(true);
        break;
    case Qt::Key_C:
        ui->b_vc->setChecked(true);
        break;
    case Qt::Key_B:
        ui->b_vb->setChecked(true);
        break;
        }
        break;
    }
    _heldKey = Qt::Key_Escape;
}
