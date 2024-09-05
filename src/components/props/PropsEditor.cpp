//
// Created by benichn on 13/08/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_PropsEditor.h" resolved

#include "PropsEditor.h"
#include "ui_PropsEditor.h"
#include "../../app.h"
#include "../../data/PageSettingsModel.h"
#include "../../data/StaticJsonDelegate.h"


void PropsEditor::setSource(const PropsSource& source)
{
    _source = source;
    updateSource();
}

PropsEditor::PropsEditor(QWidget* parent) :
    QWidget(parent), ui(new Ui::PropsEditor)
{
    ui->setupUi(this);
    ui->treeView->setModel(new PageSettingsModel(this));
    ui->treeView->setItemDelegate(new StaticJsonDelegate(this));
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->lastErrorLabel->hide();
    connect(qobject_cast<StaticJsonModel*>(ui->treeView->model()), &StaticJsonModel::jsonReset, [=]
    {
        ui->treeView->expandAll();
    });
    connect(ui->modeSelector, &QComboBox::currentIndexChanged, [=](const auto i)
    {
        const auto mode = static_cast<PageColorMode>(i);
        switch (_source.index())
        {
        case PTY_PAGE:
            app().book().setPageMode(get<PTY_PAGE>(_source), mode);
            break;
        }
        updateMode(mode);
    });
    connect(&app().book(), &Book::pageLastErrorChanged, [=](const int id)
    {
        if (_source.index() == PTY_PAGE && get<PTY_PAGE>(_source) == id)
        {
            updateLastError(app().book().page(id).lastError);
        }
    });
    updateSource();
}

PropsEditor::~PropsEditor()
{
    delete ui;
}

void PropsEditor::updateSource()
{
    switch (_source.index())
    {
    case PTY_NONE:
        hide();
        break;
    case PTY_PAGE:
        {
            show();
            const auto& book = app().book();
            const int id = get<PTY_PAGE>(_source);
            const auto& page = book.page(id);
            ui->bwEdit->setPlainText(page.source.c_str());
            const auto mode = page.colorMode;
            ui->modeSelector->setCurrentIndex(mode);
            updateMode(mode);
            if (book.page(id).colorMode != PT_BLACK)
            {
                ui->cgEdit->setPlainText(page.cgSource.value().c_str());
            }
            updateLastError(page.lastError);
        }
        break;
    }
    const auto model = qobject_cast<PageSettingsModel*>(ui->treeView->model());
    model->setSource(_source);
}

void PropsEditor::updateLastError(const optional<string> lastError)
{
    if (lastError.has_value())
    {
        ui->lastErrorLabel->show();
        ui->lastErrorLabel->setPlainText(lastError.value().c_str());
    }
    else
    {
        ui->lastErrorLabel->hide();
        ui->lastErrorLabel->setPlainText("");
    }
}

void PropsEditor::updateMode(const PageColorMode mode)
{
    const auto cg = mode != PT_BLACK;
    ui->cgEdit->setVisible(cg);
    ui->cgLabel->setVisible(cg);
}
