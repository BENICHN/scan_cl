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
        case TSS_NONE:
            painter.setBrush(Qt::white);
            break;
        case TSS_WORKING:
            painter.setBrush(Color::orange);
            break;
        case TSS_WAITING:
            painter.setBrush(Color::sunFlower);
            break;
        case TSS_COMPLETED:
            painter.setBrush(Color::emerald);
            break;
        case TSS_OLD:
            painter.setBrush(Color::nephritis);
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
        const Page* page = App::instance()->database()->book()->get(_pageId);
        switch (page->type)
        {
        case PT_BLACK:
            _steps = {
                {PS_CROPPING, "Recadrage", TSS_NONE},
                {PS_CLEANING, "Nettoyage", TSS_NONE},
                {PS_FINAL, "Finalisation", TSS_NONE}
            };
        case PT_COLOR:
        case PT_GRAY:
            _steps = {
                {PS_CROPPING, "Recadrage", TSS_NONE},
                {PS_MERGING, "Assemblage", TSS_NONE},
                {PS_CLEANING, "Nettoyage", TSS_NONE},
                {PS_FINAL, "Finalisation", TSS_NONE}
            };
        }
        int currentStepIndex = -1;
        if (page->lastStep.step != PS_NONE)
        {
            for (TimelineStep& step : _steps)
            {
                currentStepIndex++;
                step.status = TSS_COMPLETED;
                if (step.id == page->lastStep.step) break;
            }
        }
        switch (page->status)
        {
        case PST_IDLE:
            break;
        case PST_WORKING:
            _steps[currentStepIndex+1].status = TSS_WORKING;
            break;
        case PST_WAITING:
            _steps[currentStepIndex+1].status = TSS_WAITING;
            break;
        }
    }
    update();
}
