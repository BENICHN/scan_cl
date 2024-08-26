//
// Created by benichn on 20/08/24.
//

#ifndef SCANBUTTONS_H
#define SCANBUTTONS_H

#include <QWidget>


class ImageViewerWidget;
QT_BEGIN_NAMESPACE
namespace Ui { class ScanButtons; }
QT_END_NAMESPACE

class ScanButtons : public QWidget {
Q_OBJECT
    Qt::Key _heldKey = Qt::Key_Escape;

public:
    explicit ScanButtons(QWidget *parent = nullptr);
    ~ScanButtons() override;
    [[nodiscard]] ImageViewerWidget& iv();

protected:
void keyPressEvent(QKeyEvent* event) override;

private:
    Ui::ScanButtons *ui;
};


#endif //SCANBUTTONS_H
