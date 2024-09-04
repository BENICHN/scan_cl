//
// Created by benichn on 29/07/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_PreviewerWidget.h" resolved

#include "PreviewerWidget.h"
#include "ui_PreviewerWidget.h"
#include "../app.h"


void PreviewerWidget::setEditable(const bool editable)
{
    _editable = editable;
    updateImageAndRect(false, true);
}

PreviewerWidget::PreviewerWidget(QWidget* parent) :
    QWidget(parent), ui(new Ui::PreviewerWidget)
{
    ui->setupUi(this);
    connect(&app().book(), &Book::pageStatusChanged, [=](int pageId)
    {
        if (pageId == _pageId) // !
        {
            updateSelector();
            updateImageAndRect(true, true); // !
        }
    });
    connect(&app().book(), &Book::pageModeChanged, [=](int pageId)
    {
        if (pageId == _pageId) // !
        {
            updateSelector();
            updateImageAndRect(true, true); // !
        }
    });
    connect(&app().book(), &Book::bookReset, [=]
    {
        _pageId = -1;
        updateSelector();
        updateImageAndRect(true, true); // !
    });
    connect(ui->selector, &PreviewerSelector::selectionChanged, [=]
    {
        updateImageAndRect(true, true); // !
    });
    updateSelector();
    const auto ly = qobject_cast<QGridLayout*>(layout());
    ly->removeWidget(ui->selector);
    ly->addWidget(ui->selector, 0, 0, Qt::AlignRight | Qt::AlignTop);
}

PreviewerWidget::~PreviewerWidget()
{
    delete ui;
}

void PreviewerWidget::setPageId(const int id)
{
    _pageId = id;
    updateSelector();
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
                    ui->iv->setPixmap(book.pageSourceBWPath(_pageId));
                    break;
                case ImageOrigin::GENERATED:
                    ui->iv->setPixmap(book.pageGeneratedBWPath(_pageId));
                    break;
                }
                break;
            case PWC_CG:
                switch (settings.origin.value())
                {
                case ImageOrigin::SOURCE:
                    ui->iv->setPixmap(book.pageSourceCGPath(_pageId));
                    break;
                case ImageOrigin::GENERATED:
                    ui->iv->setPixmap(book.pageGeneratedCGPath(_pageId));
                    break;
                }
                break;
            case PWC_MIX:
                ui->iv->setPixmap([=] { return app().book().pageGeneratedMixPixmap(_pageId); });
                break;
            }
        }
        if (updateRect)
        {
            if (_editable)
            {
                switch (settings.selectionType.value())
                {
                case SR_NONE:
                    ui->iv->setSRDisabled();
                    break;
                case SR_RECT:
                    ui->iv->setSRRect(settings.selection);
                    break;
                case SR_PICKER:
                    ui->iv->setSRPicker(get<vector<PickerElement>>(settings.selection.value()));
                    break;
                }
            }
            else
            {
                ui->iv->setSRDisabled();
            }
        }
    }
}

void PreviewerWidget::updateSelector()
{
    if (_pageId == -1)
    {
        ui->selector->hide();
    }
    else
    {
        ui->selector->show();
        const auto& book = app().book();
        ui->selector->setHasColor(book.page(_pageId).colorMode != PT_BLACK);
        ui->selector->setResMixed(book.pageMixedAvailable(_pageId));
    }
}

void PreviewerWidget::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Enter:
        validateChoice();
        break;
    case Qt::Key_Left:
        ui->selector->setPosition(ui->selector->position() + QPoint{-1, 0});
        break;
    case Qt::Key_Right:
        ui->selector->setPosition(ui->selector->position() + QPoint{1, 0});
        break;
    case Qt::Key_Up:
        ui->selector->setPosition(ui->selector->position() + QPoint{0, 1});
        break;
    case Qt::Key_Down:
        ui->selector->setPosition(ui->selector->position() + QPoint{0, -1});
        break;
    default:
        break;
    }
    QWidget::keyPressEvent(event);
}
