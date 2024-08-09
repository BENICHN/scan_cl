//
// Created by benichn on 29/07/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_PreviewerWidget.h" resolved

#include "PreviewerWidget.h"
#include "ui_PreviewerWidget.h"
#include "../app.h"


PreviewerWidget::PreviewerWidget(QWidget* parent) :
    QWidget(parent), ui(new Ui::PreviewerWidget)
{
    ui->setupUi(this);
    connect(&app().book(), &Book::pageStatusChanged, [=](int pageId)
    {
        if (pageId == _pageId) // !
        {
            updateImageAndRect(true, true); // !
        }
    });
    connect(ui->selector, &PreviewerSelector::selectionChanged, [=]
    {
        updateImageAndRect(true, true); // !
    });
}

PreviewerWidget::~PreviewerWidget()
{
    delete ui;
}

void PreviewerWidget::setPageId(const int id)
{
    _pageId = id;
    updateImageAndRect(true, true);
}

// void PreviewerWidget::setAskSettings(const AskSettings askSettings)
// {
//     const bool colorChanged = _askSettings.color != askSettings.color;
//     const bool selectionTypeChanged = _askSettings.selectionType != askSettings.selectionType;
//     _askSettings = askSettings;
//     if (colorChanged) updateImage();
//     if (selectionTypeChanged) updateRect();
// }

void PreviewerWidget::validateChoice()
{
    app().book().applyChoiceToPage(_pageId, ui->iv->sr()->selection());
}

PreviewerSettings PreviewerWidget::previewerSettings() const
{
    if (_pageId == -1)
    {
        return {};
    }
    const auto& book = app().book();
    const PreviewerSelection selection = ui->selector->selection();
    switch (selection.mode)
    {
    case PWM_SRC:
        return {
            ImageOrigin::SOURCE,
            selection.color,
            SR_NONE
        };
    case PWM_ASK:
        {
            const auto& page = book.page(_pageId);
            return page.defaultPreviewerSettings();
        }
    case PWM_RES:
        return {
            ImageOrigin::GENERATED,
            selection.color,
            SR_NONE
        };
    }
    throw runtime_error("enum selection.mode is invalid");
}

void PreviewerWidget::updateImageAndRect(const bool updateImage, const bool updateRect)
{
    if (_pageId == -1)
    {
        ui->iv->setPixmap(QPixmap());
        ui->iv->setSRDisabled();
    }
    else
    {
        const auto& book = app().book();
        const auto settings = previewerSettings();
        if (updateImage)
        {
            switch (settings.color.value())
            {
            case PWC_BW:
                switch (settings.origin.value())
                {
                case ImageOrigin::SOURCE:
                    ui->iv->setPixmap(QPixmap(book.pageSourceBWPath(_pageId).c_str()));
                    break;
                case ImageOrigin::GENERATED:
                    ui->iv->setPixmap(QPixmap(book.pageGeneratedBWPath(_pageId).c_str()));
                    break;
                }
                break;
            case PWC_CG:
                switch (settings.origin.value())
                {
                case ImageOrigin::SOURCE:
                    ui->iv->setPixmap(QPixmap(book.pageSourceCGPath(_pageId).c_str()));
                    break;
                case ImageOrigin::GENERATED:
                    ui->iv->setPixmap(QPixmap(book.pageGeneratedCGPath(_pageId).c_str()));
                    break;
                }
                break;
            case PWC_MIX:
                ui->iv->setPixmap(book.pageGeneratedMixPixmap(_pageId));
                break;
            }
        }
        if (updateRect)
        {
            switch (settings.selectionType.value())
            {
            case SR_NONE:
                ui->iv->setSRDisabled();
                break;
            case SR_RECT:
                if (settings.selection.has_value()) ui->iv->setSRRect(get<QImage>(settings.selection.value()));
                else ui->iv->setSRRect();
                break;
            case SR_PICKER:
                ui->iv->setSRPicker(get<vector<PickerElement>>(settings.selection.value()));
                break;
            }
        }
    }
}

void PreviewerWidget::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Enter:
        {
            validateChoice();
        }
        break;
    default:
        break;
    }
    QWidget::keyPressEvent(event);
}
