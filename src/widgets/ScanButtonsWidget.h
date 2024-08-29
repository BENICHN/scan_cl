//
// Created by benichn on 27/08/24.
//

#ifndef SCANBUTTONSWIDGET_H
#define SCANBUTTONSWIDGET_H

#include "ScanButtonsTypes.h"
#include "../imports/qtimports.h"

class ScanButtonsWidget final : public QWidget
{
    Q_OBJECT
    ScanButtonPanel _panel;
    Qt::Key _heldKey = Qt::Key_Escape;

public:
    bool handleKeyEvents = false;
    [[nodiscard]] ScanButtonPanel panel() const { return _panel; }
    void setPanel(const ScanButtonPanel& panel);
    explicit ScanButtonsWidget(QWidget* parent);
    void processKey(Qt::Key key);
    [[nodiscard]] bool isChecked(const string& groupName, const string& name) const;

private:
    void updateButtons();
    void keyPressEvent(QKeyEvent* event) override;

signals:
    void buttonChecked(const ScanButtonData& data);
};


#endif //SCANBUTTONSWIDGET_H
