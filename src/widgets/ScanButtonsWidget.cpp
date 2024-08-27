//
// Created by benichn on 27/08/24.
//

#include "ScanButtonsWidget.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>

#include "ScanButton.h"

void ScanButtonsWidget::setPanel(const ScanButtonPanel& panel)
{
    _panel = panel;
    updateButtons();
}

void ScanButtonsWidget::keyPressEvent(QKeyEvent* event)
{
    if (handleKeyEvents)
    {
        processKey(static_cast<Qt::Key>(event->key()));
    }
}

ScanButtonsWidget::ScanButtonsWidget(QWidget* parent): QWidget(parent)
{
    setLayout(new QHBoxLayout(this));
}

void ScanButtonsWidget::processKey(Qt::Key key)
{
    for (const auto& coll : _panel)
    {
        for (const auto& bd : coll.buttons)
        {
            if (_heldKey == Qt::Key_Escape)
            {
                if (bd.primaryKey == key)
                {
                    if (bd.secondaryKey == Qt::Key_Escape)
                    {
                        const auto btns = findChildren<ScanButton*>(Qt::FindDirectChildrenOnly);
                        const auto btn = *str::find_if(btns, [&](const auto btn) { return btn->data().name == bd.name; });
                        if (bd.alone) btn->toggle(); else btn->setChecked(true);
                        emit buttonChecked(bd);
                    }
                    else
                    {
                        _heldKey = bd.primaryKey;
                    }
                    return;
                }
            }
            else
            {
                if (_heldKey == bd.primaryKey && key == bd.secondaryKey)
                {
                    const auto btns = findChildren<ScanButton*>(Qt::FindDirectChildrenOnly);
                    const auto btn = *str::find_if(btns, [&](const auto btn) { return btn->data().name == bd.name; });
                    if (bd.alone) btn->toggle(); else btn->setChecked(true);
                    emit buttonChecked(bd);
                    _heldKey = Qt::Key_Escape;
                    return;
                }
            }
        }
    }
    _heldKey = Qt::Key_Escape;
}

bool ScanButtonsWidget::isChecked(const string& name) const
{
    for (const auto btn : findChildren<ScanButton*>(Qt::FindDirectChildrenOnly))
    {
        if (btn->data().name == name)
        {
            return btn->isChecked();
        }
    }
    throw runtime_error("Aucun bouton dont le nom est : "+name);
}

void ScanButtonsWidget::updateButtons()
{
    const auto ly = qobject_cast<QHBoxLayout*>(layout());
    while(!ly->isEmpty()) {
        delete ly->takeAt(0);
    }
    for (const auto& coll : _panel)
    {
        const auto cly = new QVBoxLayout(this);
        ly->addLayout(cly);
        const auto lab = new QLabel(this);
        lab->setText(coll.name.c_str());
        lab->setStyleSheet("* {color: rgb(127, 127, 127); font-size: 16px;}");
        cly->addWidget(lab);
        const auto bg = new QButtonGroup(this);
        bg->setExclusive(true);
        for (const auto& bd : coll.buttons)
        {
            const auto btn = new ScanButton(this, bd);
            if (bd.checkable && !bd.alone) bg->addButton(btn);
            cly->addWidget(btn);
        }
        cly->addStretch();
    }
}
