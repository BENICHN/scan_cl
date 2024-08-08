//
// Created by benichn on 02/08/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_PreviewerSelector.h" resolved

#include "PreviewerSelector.h"
#include "ui_PreviewerSelector.h"


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
