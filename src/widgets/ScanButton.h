//
// Created by benichn on 27/08/24.
//

#ifndef SCANBUTTON_H
#define SCANBUTTON_H

#include <QPushButton>

#include "ScanButtonsTypes.h"

class ScanButton final : public QPushButton
{
    Q_OBJECT
    ScanButtonData _data;

public:
    [[nodiscard]] QSize minimumSizeHint() const override;
    ScanButton(QWidget* parent, const ScanButtonData& data);
    [[nodiscard]] const ScanButtonData& data() const { return _data; }
};


#endif //SCANBUTTON_H
