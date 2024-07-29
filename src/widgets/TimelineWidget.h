//
// Created by benichn on 12/07/24.
//

#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include "../qtimports.h"

QT_BEGIN_NAMESPACE

namespace Ui
{
    class TimelineWidget;
}

QT_END_NAMESPACE

class TimelineWidget final : public QWidget
{
    Q_OBJECT
    vector<vector<QPoint>> _lines;

public:
    explicit TimelineWidget(QWidget* parent = nullptr);
    ~TimelineWidget() override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void updateLinesAndSize();
    void resizeEvent(QResizeEvent* event) override;

private:
    Ui::TimelineWidget* ui;

public:
    static constexpr int LINE_OFFSET = 8;
    static constexpr int ARROWS_OFFSET = 12;
};


#endif //TIMELINEWIDGET_H
