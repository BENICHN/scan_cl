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
#include <QGraphicsOpacityEffect>

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

PageColorMode getColorModeFromSB(ScanButtonsWidget* sb)
{
    return sb->isChecked("Mode", "Bitonal") ? PT_BLACK : sb->isChecked("Mode", "Nuances de gris") ? PT_GRAY : PT_COLOR;
}

json getCropSettingsFromSB(ScanButtonsWidget* sb)
{
    json res = json::object();
    res["flip"] = sb->isChecked("Divers", "Retourner");
    return res;
}

json getFinalSettingsFromSB(ScanButtonsWidget* sb)
{
    json res = json::object();
    res["alignmentH"] = sb->isChecked("Alignement(H)", "Gauche")
                            ? "l"
                            : sb->isChecked("Alignement(H)", "Centre")
                            ? "c"
                            : "r";
    res["alignmentV"] = sb->isChecked("Alignement(V)", "Haut")
                            ? "t"
                            : sb->isChecked("Alignement(V)", "En-tête")
                            ? "h"
                            : sb->isChecked("Alignement(V)", "Centre")
                            ? "c"
                            : "b";
    return res;
}

ScanWidget::ScanWidget(QWidget* parent) :
    QWidget(parent), ui(new Ui::ScanWidget)
{
    ui->setupUi(this);
    ui->lIV->setEditable(false);
    ui->visButton->hide();
    ui->stopBtn->hide();

    ui->rLayout->addWidget(ui->lIcon, 0, 0, Qt::AlignRight | Qt::AlignTop);
    ui->postTEdit->setPlainText(app().appSettings().postT().c_str());
    ui->postTBox->setChecked(app().appSettings().postTEnabled());

    connect(&app().scanner(), &Scanner::devicesFound, [=] { updateDevices(); });
    connect(&app().scanner(), &Scanner::currentDeviceChanged, [=]
    {
        updateDevList();
        updatePixmap();
    });
    connect(&app().scanner(), &Scanner::isChangingDeviceChanged, [=]
    {
        updateDevListEnabled();
    });
    connect(ui->devList, &QComboBox::currentIndexChanged, [=](int i)
    {
        if (!_canUpdateDevice) return;
        auto& scanner = app().scanner();
        // if (i <= 0 && !scanner.deviceSelected()) return;
        // if (scanner.currentDeviceName() == ui->devList->currentText().toStdString()) return;
        scanner.setCurrentDevice(i - 1);;
    });
    connect(ui->pageNav->list()->selectionModel(), &QItemSelectionModel::selectionChanged, [=]
    {
        const int newId = uniqueSelectedId(ui->pageNav->list()->selectionModel());
        ui->lIV->setPageId(newId);
    });
    connect(ui->playBtn, &QPushButton::clicked, [=]
    {
        startScanning();
    });
    connect(ui->collBtn, &QPushButton::clicked, [=]
    {
        setTopCollapsed(true);
    });
    connect(ui->visButton, &QPushButton::clicked, [=]
    {
        setTopCollapsed(false);
    });
    connect(ui->stopBtn, &QPushButton::clicked, [=]
    {
        stopScanning();
    });
    connect(ui->postTEdit, &QPlainTextEdit::textChanged, [=]
    {
        app().appSettings().setPostT(ui->postTEdit->toPlainText().toStdString());
    });
    connect(ui->postTBox, &QCheckBox::toggled, [=]
    {
        app().appSettings().setPostTEnabled(ui->postTBox->isChecked());
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

    const auto modelBw = new ScanOptionsModel(ui->opts);
    modelBw->setMode(PT_BLACK);
    ui->opts->setModel(modelBw);
    ui->opts->setItemDelegate(new StaticJsonDelegate(ui->opts));
    ui->opts->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->opts->expandAll();

    const auto modelG = new ScanOptionsModel(ui->optsG);
    modelG->setMode(PT_GRAY);
    ui->optsG->setModel(modelG);
    ui->optsG->setItemDelegate(new StaticJsonDelegate(ui->optsG));
    ui->optsG->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->optsG->expandAll();

    const auto modelC = new ScanOptionsModel(ui->optsC);
    modelC->setMode(PT_COLOR);
    ui->optsC->setModel(modelC);
    ui->optsC->setItemDelegate(new StaticJsonDelegate(ui->optsC));
    ui->optsC->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->optsC->expandAll();

    const auto panel = defaultPanel();
    ui->rSB->setPanel(panel);
    // ui->lSB->setPanel(panel);
    updateDevices();
    updateDevList();
    updateDevListEnabled();

    ui->lBWLab->setGraphicsEffect(new QGraphicsOpacityEffect(this));
    ui->lCGLab->setGraphicsEffect(new QGraphicsOpacityEffect(this));
    _aBW = new QPropertyAnimation(ui->lBWLab->graphicsEffect(), "opacity");
    _aBW->setDuration(1000);
    _aBW->setKeyValueAt(0.5, 0);
    _aBW->setStartValue(1);
    _aBW->setEndValue(1);
    _aBW->setLoopCount(-1);
    _aBW->start();
    _aCG = new QPropertyAnimation(ui->lCGLab->graphicsEffect(), "opacity");
    _aCG->setDuration(1000);
    _aCG->setKeyValueAt(0.5, 0);
    _aCG->setStartValue(1);
    _aCG->setEndValue(1);
    _aCG->setLoopCount(-1);
    _aCG->start();
}

ScanWidget::~ScanWidget()
{
    delete ui;
}

void ScanWidget::updateDevices()
{
    _canUpdateDevice = false;
    ui->devList->clear();
    const auto& devs = app().scanner().devices();
    ui->devList->addItem("- Sélectionner un appareil -");
    ui->devList->addItems(devs | stv::transform([](const auto& dev) { return dev.name; }) | str::to<QStringList>());
    ui->devList->setCurrentIndex(0);
    _canUpdateDevice = true;
}

void ScanWidget::updateDevListEnabled()
{
    ui->devList->setDisabled(app().scanner().isChangingDevice());
}

void ScanWidget::updateDevList()
{
    _canUpdateDevice = false;
    const auto& name = app().scanner().currentDeviceName();
    if (name.has_value()) ui->devList->setCurrentText(name->c_str());
    else ui->devList->setCurrentIndex(0);
    _canUpdateDevice = true;
}

// void ScanWidget::updateOptions()
// {
//     // return app().scanner().getOptionsValues().then([=](const auto& j)
//     // {
//     //     ui->opts->setText(j ? j.value().dump(2).c_str() : "");
//     // });
// }

Task<bool> ScanWidget::updatePixmap(const char* savingPath)
{
    auto pix = app().scanner().generateCurrentPixmap();
    // addCropLines(pix);
    ui->rIV->setPixmap(pix);
    if (savingPath)
    {
        co_return co_await QtConcurrent::run([&]
        {
            return pix.save(savingPath);
        });
    }
    co_return true;
}

Task<> ScanWidget::updatePixmapLoop()
{
    const auto& scanner = app().scanner();
    while (scanner.scanning())
    {
        co_await updatePixmap();
        co_await delay(200);
    }
}

void ScanWidget::setScanningIcon(const optional<PageColorMode>& mode)
{
    ui->lBWLab->setPixmap({});
    ui->lCGLab->setPixmap({});
    if (mode.has_value())
    {
        ui->lBWLab->setPixmap(QPixmap(":/icons/sB.svg"));
        const auto m = mode.value();
        if (m != PT_BLACK)
        {
            _aBW->stop();
            ui->lBWLab->graphicsEffect()->setProperty("opacity", 1);
            _aCG->start();
            switch (m)
            {
            case PT_COLOR:
                ui->lCGLab->setPixmap(QPixmap(":/icons/sC.svg"));
                break;
            case PT_GRAY:
                ui->lCGLab->setPixmap(QPixmap(":/icons/sG.svg"));
                break;
            }
        }
        else
        {
            _aBW->start();
        }
    }
}

void ScanWidget::setTopCollapsed(const bool collapsed)
{
    const auto l = layout();
    for (int i = 0; i < l->count() - 2; ++i)
    {
        const auto ly = l->itemAt(i)->layout();
        for (const auto it : recursiveLayoutChildren(ly))
        {
            // const auto it = ly->itemAt(i);
            if (it->widget())
            {
                it->widget()->setHidden(collapsed);
            }
        }
    }
    ui->visButton->setVisible(collapsed);
}

void ScanWidget::startScanning()
{
    if (!_scanning)
    {
        _scanning = true;
        setTopCollapsed(true);
        ui->playBtn->hide();
        ui->stopBtn->show();
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
    const auto& settings = app().appSettings();

    bool scanningColor = false;
    path imagePath;
    path imageColorPath;
    PageColorMode mode = PT_BLACK;

    while (_scanning)
    {
        if (!scanningColor) // indique une nouvelle page
        {
            imagePath = book.getNewScanPath();
            imageColorPath = imagePath.parent_path() / (imagePath.stem().string() + "-color" + imagePath.extension().
                string());
        }
        const auto& md = scanningColor ? mode : PT_BLACK;
        setScanningIcon(md);
        scanner.setOptionValues(settings.getRealScanOptions(md));
        const auto sta = co_await scanner.startScan();
        if (!sta)
        {
            remove(imagePath);
            remove(imageColorPath);
            stopScanning();
            break;
        }
        while (scanner.scanning())
        {
            co_await updatePixmap();
            co_await delay(200);
        }
        if (!_scanning || scanner.imageEmpty())
        {
            continue;
        }
        mode = getColorModeFromSB(ui->rSB);
        const auto sclr = mode != PT_BLACK;
        if (scanningColor && sclr)
        {
            if (!co_await updatePixmap(imageColorPath.c_str()))
            {
                remove(imagePath);
                remove(imageColorPath);
                stopScanning();
                break;
            }
        }
        if (!scanningColor)
        {
            if (!co_await updatePixmap(imagePath.c_str()))
            {
                remove(imagePath);
                remove(imageColorPath);
                stopScanning();
                break;
            }
        }

        const auto sameSrc = mode == PT_GRAY && !ui->cbGray->isChecked() || mode == PT_COLOR && !ui->cbColor->
            isChecked();

        if (scanningColor || !sclr || sameSrc) // indique prochainement une nouvelle page
        {
            if (ui->postTBox->isChecked())
            {
                QProcess p;
                ostringstream s;
                s << "bw=" << imagePath << ";cg=" << imageColorPath << ";" << ui->postTEdit->toPlainText().toStdString();
                p.startCommand(s.str().c_str());
                co_await qCoro(p).waitForFinished(60000);
            }
            auto page = Page{
                std::rand(), mode, imagePath.filename(),
                sclr ? sameSrc ? optional(imagePath.filename()) : optional(imageColorPath.filename()) : nullopt, 1
            };
            page.croppingStep().settings.update(getCropSettingsFromSB(ui->rSB));
            page.finalStep().settings.update(getFinalSettingsFromSB(ui->rSB));
            book.insertPageBack(std::move(page));
            app().works().enqueue({page.id, true, true});
            ui->pageNav->selectLastPage();
            scanner.clearCurrentImage();
            co_await updatePixmap();
            if (!ui->scanMultBox->isChecked())
            {
                stopScanning();
                break;
            }
        }
        co_await delay(ui->delayBox->value() * 1000);
        scanningColor = !scanningColor && sclr;
    }
    setScanningIcon(nullopt);
    ui->playBtn->show();
    ui->stopBtn->hide();
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
