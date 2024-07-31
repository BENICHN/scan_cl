//
// Created by benichn on 12/07/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_TimelineWidget.h" resolved

#include "TimelineWidget.h"
#include "ui_TimelineWidget.h"
#include "../app.h"
#include "../colors.h"
#include "../utils.h"

TimelineWidget::TimelineWidget(QWidget* parent) :
    QWidget(parent), ui(new Ui::TimelineWidget)
{
    ui->setupUi(this);
    connect(app(), &App::pageStatusChanged, [this](int pageId)
    {
        if (pageId == _pageId)
        {
            updateSteps();
        }
    });
    // ui->stepAssembl->setStyleSheet(R"(
    //     QPushButton {
    //         background-color: red;
    //     }
    // )");
}

TimelineWidget::~TimelineWidget()
{
    delete ui;
}

void TimelineWidget::setPageId(int id)
{
    _pageId = id;
    updateSteps();
}

void TimelineWidget::paintEvent(QPaintEvent* event)
{
    if (_steps.size() == 0) return;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const int w = width();
    const int h = height();
    painter.setClipRegion(roundedRectInner(0, 0, w, h, RADIUS));
    painter.setBrush(Qt::white);
    const float xStep = w / _steps.size();
    const int cy = h / 2;
    for (int i = 0; i < _steps.size(); ++i)
    {
        // make poly
        QPolygon poly;
        if (i == 0)
        {
            poly
                << QPoint{0, 0}
                << QPoint{0, h};
        }
        else
        {
            const int x = i * xStep;
            poly
                << QPoint{x - ARROW_RADIUS, 0}
                << QPoint{x + ARROW_RADIUS, cy}
                << QPoint{x - ARROW_RADIUS, h};
        }
        if (i == _steps.size() - 1)
        {
            poly
                << QPoint{w, h}
                << QPoint{w, 0};
        }
        else
        {
            const int x = (i + 1) * xStep;
            poly
                << QPoint{x - ARROW_RADIUS, h}
                << QPoint{x + ARROW_RADIUS, cy}
                << QPoint{x - ARROW_RADIUS, 0};
        }
        // fill polygons
        const TimelineStep step = _steps[i];
        painter.setPen(Qt::NoPen);
        switch (step.status)
        {
        case PST_READY:
            painter.setBrush(Qt::white);
            break;
        case PST_WORKING:
            painter.setBrush(Color::orange);
            break;
        case PST_WAITING:
            painter.setBrush(Color::sunFlower);
            break;
        case PST_COMPLETED:
            painter.setBrush(Color::emerald);
            break;
        case PST_ERROR:
            painter.setBrush(Color::alizarin);
            break;
        }
        painter.drawPolygon(poly);
        // draw lines
        painter.setPen(Qt::SolidLine);
        auto font = painter.font();
        font.setPointSize(16);
        painter.setFont(font);
        if (i > 0) { painter.drawPolyline(poly.data(), 3); }
        // draw text
        const int tx = (i + .5f) * xStep;
        drawText(painter, tx, cy, Qt::AlignCenter, step.name.c_str());
    }
    QWidget::paintEvent(event);
}

void TimelineWidget::updateSteps()
{
    if (_pageId == -1)
    {
        _steps = {};
    }
    else
    {
        const Page* page = app()->book()->get(_pageId);
        switch (page->colorMode)
        {
        case PT_BLACK:
            _steps = {
                {PS_CROPPING, "Recadrage", PST_READY},
                {PS_CLEANING, "Nettoyage", PST_READY},
                {PS_FINAL, "Finalisation", PST_READY}
            };
        case PT_COLOR:
        case PT_GRAY:
            _steps = {
                {PS_CROPPING, "Recadrage", PST_READY},
                {PS_MERGING, "Assemblage", PST_READY},
                {PS_CLEANING, "Nettoyage", PST_READY},
                {PS_FINAL, "Finalisation", PST_READY}
            };
        }
        int i = 0;
        if (page->lastStep)
        {
            for (TimelineStep& step : _steps)
            {
                i++;
                step.status = PST_COMPLETED;
                if (step.id == page->lastStep) break;
            }
        }
        _steps[i].status = page->status;
    }
    update();
}
