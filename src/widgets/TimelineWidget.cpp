//
// Created by benichn on 12/07/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_TimelineWidget.h" resolved

#include "TimelineWidget.h"
#include "ui_TimelineWidget.h"
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

void TimelineWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    for (const auto& polyline : _lines)
    {
        painter.drawPolyline(polyline.data(), polyline.size());
        const QPoint* avd = &polyline[polyline.size() - 2];
        drawArrow(&painter, avd[1], atan2(avd[1].y() - avd[0].y(), avd[1].x() - avd[0].x()), 6);
    }
    QWidget::paintEvent(event);
}

void TimelineWidget::updateLinesAndSize()
{
    const QRect sourceBounds = ui->source->geometry();
    const QRect endBounds = ui->end->geometry();
    const QRect step1Bounds = ui->step1->geometry();
    const QRect stepAssemblBounds = ui->stepAssembl->geometry();
    const QRect step2Bounds = ui->step2->geometry();
    const QRect stepFinalBounds = ui->stepFinal->geometry();

    ui->source->setFixedWidth(ui->source->geometry().width());
    ui->end->setFixedWidth(ui->end->geometry().width());
    ui->step1->setFixedHeight(sourceBounds.height());
    const float endHeight = endBounds.height();
    ui->stepAssembl->setFixedHeight(endHeight);
    ui->step2->setFixedHeight(endHeight);
    ui->stepFinal->setFixedHeight(endHeight);

    _lines = {
        {
            {sourceBounds.right() + LINE_OFFSET, sourceBounds.top() + ARROWS_OFFSET},
            {step1Bounds.left() - LINE_OFFSET, sourceBounds.top() + ARROWS_OFFSET}
        },
        {
            {sourceBounds.right() + LINE_OFFSET, sourceBounds.bottom() - ARROWS_OFFSET},
            {step1Bounds.left() - LINE_OFFSET, sourceBounds.bottom() - ARROWS_OFFSET}
        },
        {
            {step1Bounds.right() + LINE_OFFSET, sourceBounds.top() + ARROWS_OFFSET},
            {cx(stepAssemblBounds), sourceBounds.top() + ARROWS_OFFSET},
            {cx(stepAssemblBounds), endBounds.top() - LINE_OFFSET},
        },
        {
            {step1Bounds.right() + LINE_OFFSET, sourceBounds.bottom() - ARROWS_OFFSET},
            {cx(stepAssemblBounds), sourceBounds.bottom() - ARROWS_OFFSET},
            {cx(stepAssemblBounds), endBounds.bottom() + LINE_OFFSET},
        },
        {
            {stepAssemblBounds.right() + LINE_OFFSET, cy(stepAssemblBounds)},
            {step2Bounds.left() - LINE_OFFSET, cy(step2Bounds)},
        },
        {
            {step2Bounds.right() + LINE_OFFSET, cy(step2Bounds)},
            {stepFinalBounds.left() - LINE_OFFSET, cy(stepFinalBounds)},
        },
        {
            {stepFinalBounds.right() + LINE_OFFSET, cy(stepFinalBounds)},
            {endBounds.left() - LINE_OFFSET, cy(endBounds)},
        }
    };
}

void TimelineWidget::resizeEvent(QResizeEvent* event)
{
    updateLinesAndSize();
    QWidget::resizeEvent(event);
}
