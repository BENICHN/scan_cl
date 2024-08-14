//
// Created by benichn on 12/07/24.
//

#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include "../imports/qtimports.h"
#include "../data/book.h"

QT_BEGIN_NAMESPACE

namespace Ui
{
    class TimelineWidget;
}

QT_END_NAMESPACE

class TimelineWidget final : public QWidget
{
    Q_OBJECT
    int _pageId = -1;

public:
    explicit TimelineWidget(QWidget* parent = nullptr);
    ~TimelineWidget() override;

    [[nodiscard]] int pageId() const { return _pageId; }
    void setPageId(int id);

private:
    void paintEvent(QPaintEvent* event) override;

    Ui::TimelineWidget* ui;

public:
    static constexpr int ARROW_RADIUS = 10;
    static constexpr int RADIUS = 15;
};


#endif //TIMELINEWIDGET_H
