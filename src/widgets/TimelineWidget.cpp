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
    painter.translate(8, 8);
    const int w = width()-16;
    const int h = height()-16;
    const auto rr = roundRect({0, 0, w, h}, RADIUS);
    painter.setClipPath(rr);
    const float xStep = w / sz;
    const int cy = h / 2;
    int i = 0;
    for (const auto& step : steps)
    {
        if (!step->enabled()) continue;
        painter.save();
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
        auto font = painter.font();
        font.setPixelSize(16);
        painter.setPen(Qt::NoPen);
        switch (step->status)
        {
        case SST_NOTRUN:
            painter.setBrush(Qt::white);
            break;
        case SST_WORKING:
            painter.setBrush(Qt::white);
            font.setBold(true);
            break;
        case SST_WAITING:
            painter.setBrush(Color::br3);
            break;
        case SST_COMPLETE:
            painter.setBrush(Color::br4);
            break;
        case SST_ERROR:
            painter.setBrush(Color::br_red);
            break;
        }
        painter.drawPolygon(poly);
        // draw lines
        painter.setPen(Qt::SolidLine);
        if (i > 0)
        {
            painter.save();
            painter.setPen({Color::br5, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin});
            painter.drawPolyline(poly.data(), 3);
            painter.restore();
        }
        // draw text
        painter.setFont(font);
        const int tx = (i + .5f) * xStep;
        drawText(painter, tx, cy, Qt::AlignCenter, step->name().c_str());
        painter.restore();
        if (step->enabled()) ++i;
    }
    // painter.setClipping(false);
    // painter.setPen({Color::br5, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin});
    // painter.drawPath(rr);
}
