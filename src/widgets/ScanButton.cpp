//
// Created by benichn on 27/08/24.
//

#include "ScanButton.h"

#include <QHBoxLayout>
#include <QLabel>

QSize ScanButton::minimumSizeHint() const
{
    return {0, 48};
}

ScanButton::ScanButton(QWidget* parent, const ScanButtonData& data): QPushButton(parent),
                                                                     _data(data)
{
    const auto ly = new QHBoxLayout(this);
    const auto l1 = new QLabel(this);
    const auto l2 = new QLabel(this);
    const auto line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    ly->addWidget(l1);
    ly->addWidget(line);
    ly->addWidget(l2);
    ly->setStretch(0, 1);
    l1->setText(data.name.c_str());
    l2->setText(data.keyText().c_str());
    setCheckable(data.checkable);
    setChecked(data.checked);
}
