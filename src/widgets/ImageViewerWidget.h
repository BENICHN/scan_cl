//
// Created by benichn on 12/07/24.
//

#ifndef IMAGEVIEWERWIDGET_H
#define IMAGEVIEWERWIDGET_H

#include "../qtimports.h"
#include "../mouse/MouseAction.h"
#include "../mouse/MouseHandler.h"
#include <QLabel>
#include "SelectionRectWidget.h"

class ImageViewerWidget final : public QWidget
{
    Q_OBJECT

    class MoveMouseAction final : public MouseAction
    {
        ImageViewerWidget* _sr;
        void onMove() override;

    public:
        explicit MoveMouseAction(ImageViewerWidget* iv) : _sr(iv)
        {
        }
    };

    class MouseHandler final : public ::MouseHandler
    {
        ImageViewerWidget* _sr;
        unique_ptr<MouseAction> createMouseAction(QMouseEvent* e) override;;
        void wheelEvent(QWheelEvent* e) override;;

    public:
        MouseHandler(ImageViewerWidget* iv) : _sr(iv)
        {
        }
    } _mouseHandler{this};

    QPixmap _pixmap;
    QLabel* _imageLabel;
    SelectionRectWidget* _selectionRect;
    float _maxZoomLevel;
    float _minZoomLevel;
    float _zoomLevel;

    static int adjustPos(int L, int l, int x);

public:
    static constexpr int WHEEL_OFFSET = 40;
    static constexpr float WHEEL_FACTOR = 1.1;
    static constexpr float WHEEL_FACTOR_LOG = logf(WHEEL_FACTOR);
    explicit ImageViewerWidget(QWidget* parent = nullptr);

    void offset(const QPoint& v) const;
    void zoomToLevel(float level, const QPointF& p);

    void setPixmap(const QPixmap& pixmap);
    void setPixmapAndPicker(const QPixmap& pixmap, const vector<PickerElement>& elements);

protected:
    void updateSelectionRect() const;
    void computeZoomLevels();
    bool event(QEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
};

#endif //IMAGEVIEWERWIDGET_H
