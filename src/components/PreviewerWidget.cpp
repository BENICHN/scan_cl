//
// Created by benichn on 29/07/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_PreviewerWidget.h" resolved

#include "PreviewerWidget.h"
#include "ui_PreviewerWidget.h"
#include "../app.h"


PreviewerWidget::PreviewerWidget(QWidget* parent) :
    QWidget(parent), ui(new Ui::PreviewerWidget)
{
    ui->setupUi(this);
    connect(ui->buttonGroup, &QButtonGroup::buttonToggled,
            [this](QAbstractButton*, const bool checked) { if (checked) { updateMode(); } });
}

PreviewerWidget::~PreviewerWidget()
{
    delete ui;
}

void PreviewerWidget::setPageId(const int id)
{
    _pageId = id;
    updateImage();
}

void PreviewerWidget::updateImage() const
{
    if (_pageId == -1)
    {
        ui->iv->setPixmap(QPixmap());
    }
    else
    {
        const Book* book = App::instance()->database()->book();
        const Page* page = book->get(_pageId);
        switch (_mode)
        {
        case SRC:
            ui->iv->setPixmap(QPixmap((book->sourcesDir + "/" + page->source).c_str()));
            break;
        case ASK:
        case RES:
            ui->iv->setPixmap(QPixmap());
            break;
        }
    }
}

void PreviewerWidget::updateMode()
{
    const PreviewerMode newMode = ui->srcButton->isChecked() ? SRC : ui->askButton->isChecked() ? ASK : RES;
    if (newMode != _mode)
    {
        _mode = newMode;
        updateImage();
    }
}
