//
// Created by benichn on 14/08/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ScanWidget.h" resolved

#include "ScanWidget.h"
#include "ui_ScanWidget.h"
#include "../app.h"
#include "../data/StaticJsonDelegate.h"
#include "../data/ScanOptionsModel.h"
#include "../widgets/ImageViewerWidget.h"

ScanButtonPanel defaultPanel()
{
    return {
        {
            "Mode", {
                {"Bitonal", true, true, false, Qt::Key_B},
                {"Nuances de gris", true, false, false, Qt::Key_G},
                {"Couelur", true, false, false, Qt::Key_C},
            }
        },
        {
            "Alignement(H)", {
                {"Gauche", true, true, false, Qt::Key_H, Qt::Key_L},
                {"Centre", true, false, false, Qt::Key_H, Qt::Key_C},
                {"Droite", true, false, false, Qt::Key_H, Qt::Key_R},
            }
        },
        {
            "Alignement(V)", {
                {"Haut", true, true, false, Qt::Key_V, Qt::Key_T},
                {"En-tête", true, false, false, Qt::Key_V, Qt::Key_H},
                {"Centre", true, false, false, Qt::Key_V, Qt::Key_T},
                {"Bas", true, false, false, Qt::Key_V, Qt::Key_B},
            }
        },
        {
            "Divers", {
                {"Retourner", true, false, true, Qt::Key_F},
                {"Marquer", true, false, true, Qt::Key_M},
                {"Refaire", false, false, false, Qt::Key_Delete},
            }
        },
    };
}

ScanWidget::ScanWidget(QWidget* parent) :
    QWidget(parent), ui(new Ui::ScanWidget)
{
    ui->setupUi(this);
    ui->stopButton->hide();
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
        startScanning();
    });
    connect(ui->stopButton, &QPushButton::clicked, [=]
    {
        stopScanning();
    });
    connect(&app().scanner(), &Scanner::pageScanned, [=]
    {
        updatePixmap();
        ui->playBtn->setEnabled(true);
    });
    connect(&app().scanner(), &Scanner::currentParametersChanged, [=]
    {
        updatePixmap();
    });
    // connect(ui->initCropW, &QSpinBox::valueChanged, [=]
    // {
    //     updatePixmap();
    // });
    // connect(ui->initCropH, &QSpinBox::valueChanged, [=]
    // {
    //     updatePixmap();
    // });
    ui->opts->setModel(new ScanOptionsModel(ui->opts));
    ui->opts->setItemDelegate(new StaticJsonDelegate(ui->opts));
    ui->opts->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->opts->expandAll();
    const auto panel = defaultPanel();
    ui->rSB->setPanel(panel);
    ui->lSB->setPanel(panel);
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
    ui->devList->addItems(devs | stv::transform([](const auto& dev) { return dev.name; }) | str::to<QStringList>());
    ui->devList->setCurrentIndex(0);
}

// void ScanWidget::updateOptions()
// {
//     // return app().scanner().getOptionsValues().then([=](const auto& j)
//     // {
//     //     ui->opts->setText(j ? j.value().dump(2).c_str() : "");
//     // });
// }

bool ScanWidget::updatePixmap(const char* savingPath)
{
    auto pix = app().scanner().generateCurrentPixmap();
    // addCropLines(pix);
    ui->rIV->setPixmap(pix);
    if (savingPath) return pix.save(savingPath);
    return true;
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

void ScanWidget::setAuxVisible(bool visible)
{
    const auto l = layout();
    for (int i = 0; i < l->count() - 1; ++i)
    {
        const auto ly = l->itemAt(i)->layout();
        for (int i = 0; i < ly->count(); ++i)
        {
            const auto it = ly->itemAt(i);
            if (it->widget())
            {
                it->widget()->setVisible(visible);
            }
        }
    }
    ui->stopButton->setHidden(visible);
}

void ScanWidget::startScanning()
{
    if (!_scanning)
    {
        _scanning = true;
        setAuxVisible(false);
        scanLoop();
    }
}

void ScanWidget::stopScanning()
{
    _scanning = false;
    app().scanner().stopScan();
}

Task<> ScanWidget::scanLoop()
{
    auto& book = app().book();
    auto& scanner = app().scanner();

    bool scanningColor = false;
    string imagePath;
    string imageColorPath;

    while (_scanning)
    {
        if (!scanningColor) // indique une nouvelle page
        {
            imagePath = book.getNewScanPath();
            auto path = stf::path(imagePath);
            imageColorPath = path.parent_path() / (path.stem().string() + "-color" + path.extension().string());
        }

        const auto sta = co_await scanner.startScan();
        if (!sta)
        {
            std::remove(imagePath.c_str());
            std::remove(imageColorPath.c_str());
            stopScanning();
            break;
        }
        while (scanner.scanning())
        {
            updatePixmap();
            co_await delay(200);
        }
        if (!_scanning || scanner.imageEmpty())
        {
            continue;
        }
        const auto sclr = !ui->rSB->isChecked("Bitonal");
        if (scanningColor && sclr)
        {
            if (!updatePixmap(imageColorPath.c_str()))
            {
                std::remove(imagePath.c_str());
                std::remove(imageColorPath.c_str());
                stopScanning();
                break;
            }
        }
        if (!scanningColor)
        {
            if (!updatePixmap(imagePath.c_str()))
            {
                std::remove(imagePath.c_str());
                std::remove(imageColorPath.c_str());
                stopScanning();
                break;
            }
        }

        if (scanningColor || !sclr) // indique prochainement une nouvelle page
        {
            if (!ui->scanMultBox->isChecked())
            {
                stopScanning();
                break;
            }
        }
        co_await delay(ui->delayBox->value() * 1000);
        scanningColor = !scanningColor && sclr;
    }
    setAuxVisible(true);
    co_return;
}

void ScanWidget::keyPressEvent(QKeyEvent* event)
{
    ui->rSB->processKey(static_cast<Qt::Key>(event->key()));
    // QWidget::keyPressEvent(event^);
}

// void ScanWidget::addCropLines(QPixmap& pix)
// {
//     const auto w = ui->initCropW->value();
//     const auto h = ui->initCropH->value();
//     QPainter painter(&pix);
//     painter.setPen({Qt::red, 1.5});
//     painter.drawPolyline(QPolygon{
//         {0, h},
//         {w, h},
//         {w, 0}
//     });
// }
