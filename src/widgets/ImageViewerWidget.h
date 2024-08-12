//
// Created by benichn on 12/07/24.
//

#ifndef IMAGEVIEWERWIDGET_H
#define IMAGEVIEWERWIDGET_H

#include "../imports/qtimports.h"
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
    optional<FilenameWithTimestamp> _ft;
    CancelToken* _imageLoadingToken = nullptr;
    QLabel* _loadingLabel;
    SelectionRectSettings _srSettings;

public:
    [[nodiscard]] const SelectionRectWidget* sr() const { return _selectionRect; }

private:
    float _maxZoomLevel;
    float _minZoomLevel;
    float _zoomLevel;

    static int adjustPos(int L, int l, int x);
    bool isCurrentFt(const string& filename, const stf::file_time_type& time);
    bool isCurrentFt(const FilenameWithTimestamp& ft);

public:
    // using ImageSource = variant<string, QPixmap>;
    static constexpr int WHEEL_OFFSET = 40;
    static constexpr float WHEEL_FACTOR = 1.1;
    static constexpr float WHEEL_FACTOR_LOG = logf(WHEEL_FACTOR);
    explicit ImageViewerWidget(QWidget* parent = nullptr);

    void offset(const QPoint& v) const;
    void zoomToLevel(float level, const QPointF& p);

    Task<> setPixmap(const string& filename);
    Task<> setPixmap(const auto& factory);
    void setPixmap(const QPixmap& pixmap);

    void setSRDisabled();
    void setSRRect();
    void setSRRect(const QImage& sel);
    void setSRRect(const optional<SelectionInfo>& sel);
    void setSRPicker(const vector<PickerElement>& elements);

    [[nodiscard]] bool isLoading() const;

protected:
    void setSR();
    Task<> setPixmapInternal(const auto& factory);
    void setPixmapInternal(const QPixmap& pixmap);
    void updateSelectionRect() const;
    void computeZoomLevels();
    void computeZoomLevels(const QSize& imgSize);
    bool event(QEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
};

Task<> ImageViewerWidget::setPixmap(const auto& factory)
{
    _ft = nullopt;
    co_await setPixmapInternal(factory);
}

Task<> ImageViewerWidget::setPixmapInternal(const auto& factory)
{
    setPixmapInternal({});
    _loadingLabel->show();
    if (_imageLoadingToken) _imageLoadingToken->cancel();
    CancelToken tk;
    _imageLoadingToken = &tk;
    const auto img = co_await QtConcurrent::run([=]
    {
        return factory();
    });
    if (tk.isCanceled()) co_return;
    _imageLoadingToken = nullptr;
    setPixmapInternal(img);
}

#endif //IMAGEVIEWERWIDGET_H
