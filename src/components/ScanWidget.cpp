//
// Created by benichn on 14/08/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ScanWidget.h" resolved

#include "ScanWidget.h"
#include "ui_ScanWidget.h"
#include "../app.h"
#include "../data/StaticJsonDelegate.h"
#include "../data/ScanOptionsModel.h"


ScanWidget::ScanWidget(QWidget* parent) :
    QWidget(parent), ui(new Ui::ScanWidget)
{
    ui->setupUi(this);
    connect(&app().scanner(), &Scanner::devicesFound, [=] { updateDevices(); });
    connect(ui->devList, &QComboBox::currentIndexChanged, [=](int i)
    {
        auto& scanner = app().scanner();
        if (i <= 0 && !scanner.deviceSelected()) return;
        ui->devList->setDisabled(true);
        scanner.setCurrentDevice(i - 1).then([=](const auto& sta)
        {
            if (!sta)
            {
                ui->devList->setCurrentIndex(0);
            }
            // updateOptions().then([=]
            // {
            updatePixmap();
            ui->devList->setEnabled(true);
            // });
        });
    });
    connect(ui->playBtn, &QPushButton::clicked, [=]
    {
        ui->playBtn->setDisabled(true);
        app().scanner().startScan().then([=](const auto& sta)
        {
            if (sta)
            {
                updatePixmapLoop();
            }
            else
            {
                ui->playBtn->setEnabled(true);
            }
        });
    });
    connect(&app().scanner(), &Scanner::pageScanned, [=]
    {
        updatePixmap();
        ui->playBtn->setEnabled(true);
    });
    ui->opts->setModel(new ScanOptionsModel(ui->opts));
    ui->opts->setItemDelegate(new StaticJsonDelegate(ui->opts));
    ui->opts->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->opts->expandAll();
    updateDevices();
}

ScanWidget::~ScanWidget()
{
    delete ui;
}

void ScanWidget::updateDevices()
{
    ui->devList->clear();
    const auto& devs = app().scanner().devices();
    ui->devList->addItem("- Sélectionner un appareil -");
    ui->devList->addItems(devs | stv::transform([](const auto& dev) { return dev->name; }) | str::to<QStringList>());
    ui->devList->setCurrentIndex(0);
}

// void ScanWidget::updateOptions()
// {
//     // return app().scanner().getOptionsValues().then([=](const auto& j)
//     // {
//     //     ui->opts->setText(j ? j.value().dump(2).c_str() : "");
//     // });
// }

void ScanWidget::updatePixmap()
{
    auto pix = app().scanner().generateCurrentPixmap();
    addCropLines(pix);
    ui->rLab->setPixmap(pix);
}

Task<> ScanWidget::updatePixmapLoop()
{
    const auto& scanner = app().scanner();
    while (scanner.scanning())
    {
        updatePixmap();
        co_await delay(200);
    }
}

void ScanWidget::addCropLines(QPixmap& pix)
{
    const auto w = ui->initCropW->value();
    const auto h = ui->initCropH->value();
    QPainter painter(&pix);
    painter.setPen({Qt::red, 1.5});
    painter.drawPolyline(QPolygon{
        {0, h},
        {w, h},
        {w, 0}
    });
}
