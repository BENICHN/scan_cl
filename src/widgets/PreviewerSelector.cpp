//
// Created by benichn on 02/08/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_PreviewerSelector.h" resolved

#include "PreviewerSelector.h"
#include "ui_PreviewerSelector.h"


void PreviewerSelector::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setBrush(QColor(0, 0, 0, 100));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(0, 0, width(), height(), 16, 16);
    QWidget::paintEvent(event);
}

void PreviewerSelector::setHasColor(const bool value)
{
    _hasColor = value;
    updateButtons();
}

void PreviewerSelector::setResMixed(const bool value)
{
    _resMixed = value;
    updateButtons();
}

PreviewerSelector::PreviewerSelector(QWidget* parent) :
    QWidget(parent), ui(new Ui::PreviewerSelector)
{
    ui->setupUi(this);
    connect(ui->buttonGroup, &QButtonGroup::buttonToggled, [=]
    {
        updateSelection();
    });
    updateButtons();
}

PreviewerSelector::~PreviewerSelector()
{
    delete ui;
}

QPoint PreviewerSelector::position()
{
    switch (_selection.mode)
    {
    case PWM_SRC:
        switch (_selection.color)
        {
        case PWC_BW:
            return {0, 0};
        case PWC_CG:
            return {0, 1};
        }
        break;
    case PWM_ASK:
        return {1, 0};
    case PWM_RES:
        switch (_selection.color)
        {
        case PWC_BW:
            return {2, 0};
        case PWC_MIX:
            return {2, 1};
        case PWC_CG:
            return {2, 2};
        }
        break;
    }
}

void PreviewerSelector::setPosition(const QPoint& position)
{
    switch (position.x())
    {
    case 0:
    case 3:
        switch (position.y())
        {
        case 2:
        case 0:
            ui->srcBwButton->setChecked(true);
            break;
        case 1:
        case -1:
            ui->srcCGButton->setChecked(true);
            break;
        }
        break;
    case 1:
        ui->askButton->setChecked(true);
        break;
    case 2:
    case -1:
        if (ui->resMixButton->isVisible())
        {
            ui->resMixButton->setChecked(true);
        }
        else
        {
            switch (position.y())
            {
            case 2:
            case 0:
                ui->resBwButton->setChecked(true);
                break;
            case 1:
            case -1:
                ui->resCGButton->setChecked(true);
                break;
            }
        }
        break;
    }
}

void PreviewerSelector::updateButtons()
{
    ui->resBwButton->setVisible(true);
    if (_hasColor)
    {
        ui->srcCGButton->setVisible(true);
        if (_resMixed)
        {
            ui->resMixButton->setVisible(true);
            ui->resBwButton->setVisible(false);
            ui->resCGButton->setVisible(false);
        }
        else
        {
            ui->resMixButton->setVisible(false);
            ui->resCGButton->setVisible(true);
        }
    }
    else
    {
        ui->srcCGButton->setVisible(false);
        ui->resCGButton->setVisible(false);
        ui->resMixButton->setVisible(false);
    }
    const QAbstractButton* selected = ui->buttonGroup->checkedButton();
    if (selected->isHidden())
    {
        if (selected == ui->srcCGButton) { ui->srcBwButton->setChecked(true); }
        else if (selected == ui->resMixButton) { ui->resBwButton->setChecked(true); }
        else if (selected == ui->resBwButton || (selected == ui->resCGButton && ui->resMixButton->isVisible()))
        {
            ui->resMixButton->setChecked(true);
        }
        else if (selected == ui->resCGButton) { ui->resBwButton->setVisible(true); }
    }
}

void PreviewerSelector::updateSelection()
{
    PreviewerSelection sel;
    auto* btn = ui->buttonGroup->checkedButton();
    if (btn == ui->askButton)
    {
        sel = {
            PWM_ASK,
            PWC_BW
        };
    }
    else if (btn == ui->srcBwButton)
    {
        sel = {
            PWM_SRC,
            PWC_BW
        };
    }
    else if (btn == ui->srcCGButton)
    {
        sel = {
            PWM_SRC,
            PWC_CG
        };
    }
    else if (btn == ui->resBwButton)
    {
        sel = {
            PWM_RES,
            PWC_BW
        };
    }
    else if (btn == ui->resCGButton)
    {
        sel = {
            PWM_RES,
            PWC_CG
        };
    }
    else if (btn == ui->resMixButton)
    {
        sel = {
            PWM_RES,
            PWC_MIX
        };
    }
    if (sel != _selection)
    {
        _selection = sel;
        emit selectionChanged(sel);
    }
}
