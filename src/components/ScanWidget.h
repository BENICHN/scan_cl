//
// Created by benichn on 14/08/24.
//

#ifndef SCANWIDGET_H
#define SCANWIDGET_H

#include <QWidget>
#include "../imports/qcoroimports.h"


QT_BEGIN_NAMESPACE

namespace Ui
{
    class ScanWidget;
}

QT_END_NAMESPACE

class ScanWidget : public QWidget
{
    Q_OBJECT
    bool _scanning;
    bool _canUpdateDevice = true;

public:
    explicit ScanWidget(QWidget* parent = nullptr);
    ~ScanWidget() override;

private:
    void updateDevices();
    void updateDevListEnabled();
    void updateDevList();
    // Task<> updateOptions();
    bool updatePixmap(const char* savingPath = nullptr);
    Task<> updatePixmapLoop();

    void setAuxVisible(bool visible);
    void startScanning();
    void stopScanning();
    Task<> scanLoop();

    void keyPressEvent(QKeyEvent* event) override;

    // void addCropLines(QPixmap& pix);
    Ui::ScanWidget* ui;
};


#endif //SCANWIDGET_H
