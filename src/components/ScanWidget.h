//
// Created by benichn on 14/08/24.
//

#ifndef SCANWIDGET_H
#define SCANWIDGET_H

#include <QWidget>
#include "../imports/qcoroimports.h"


QT_BEGIN_NAMESPACE
namespace Ui { class ScanWidget; }
QT_END_NAMESPACE

class ScanWidget : public QWidget {
Q_OBJECT

public:
    explicit ScanWidget(QWidget *parent = nullptr);
    ~ScanWidget() override;

private:
    void updateDevices();
    // Task<> updateOptions();
    void updatePixmap();
    Task<> updatePixmapLoop();

protected:
void keyPressEvent(QKeyEvent* event) override;

private:
// void addCropLines(QPixmap& pix);
    Ui::ScanWidget *ui;
};


#endif //SCANWIDGET_H
