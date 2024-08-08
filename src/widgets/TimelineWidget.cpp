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
    connect(&app().book(), &Book::pageStatusChanged, [=](int pageId)
    {
        if (pageId == _pageId)
        {
            update();
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
    update();
}

void TimelineWidget::paintEvent(QPaintEvent* event)
{
    if (_pageId == -1) return;
    const auto& steps = app().book().page(_pageId).steps;
    auto sz = str::count_if(steps, [](const auto& step) { return step->enabled(); });
    if (sz == 0) return;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const int w = width();
    const int h = height();
    painter.setClipRegion(roundedRectInner(0, 0, w, h, RADIUS));
    painter.setBrush(Qt::white);
    const float xStep = w / sz;
    const int cy = h / 2;
    int i = 0;
    for (const auto& step : steps)
    {
        if (!step->enabled()) continue;
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
        if (i == sz - 1)
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
        painter.setPen(Qt::NoPen);
        switch (step->status)
        {
        case SST_NOTRUN:
            painter.setBrush(Qt::white);
            break;
        case SST_WORKING:
            painter.setBrush(Color::orange);
            break;
        case SST_WAITING:
            painter.setBrush(Color::sunFlower);
            break;
        case SST_COMPLETE:
            painter.setBrush(Color::emerald);
            break;
        case SST_ERROR:
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
        drawText(painter, tx, cy, Qt::AlignCenter, step->name().c_str());
        if (step->enabled()) ++i;
    }
    QWidget::paintEvent(event);
}
