//
// Created by benichn on 14/08/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ScanWidget.h" resolved

#include "ScanWidget.h"
#include "ui_ScanWidget.h"
#include "../app.h"


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
            updateOptions();
            ui->devList->setEnabled(true); // ! trop tot
        });
    });
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

void ScanWidget::updateOptions()
{
    app().scanner().getOptionsValues().then([=](const auto& j)
    {
        ui->opts->setText(j ? j.value().dump(2).c_str() : "");
    });
}
