//
// Created by benichn on 12/07/24.
//

#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include "../qtimports.h"
#include "../data/book.h"

QT_BEGIN_NAMESPACE

namespace Ui
{
    class TimelineWidget;
}

QT_END_NAMESPACE

enum TimelineStepStatus
{
    TSS_NONE,
    TSS_WORKING,
    TSS_WAITING,
    TSS_COMPLETED,
    TSS_OLD
};

struct TimelineStep
{
    PageStep id;
    string name;
    TimelineStepStatus status;
};

class TimelineWidget final : public QWidget
{
    Q_OBJECT
    vector<TimelineStep> _steps;
    int _pageId = -1;

public:
    explicit TimelineWidget(QWidget* parent = nullptr);
    ~TimelineWidget() override;

    void setPageId(int id);

private:
    void paintEvent(QPaintEvent* event) override;
    void updateSteps();

    Ui::TimelineWidget* ui;

public:
    static constexpr int ARROW_RADIUS = 10;
    static constexpr int RADIUS = 15;
};


#endif //TIMELINEWIDGET_H
