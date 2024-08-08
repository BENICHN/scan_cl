//
// Created by benichn on 12/07/24.
//

#ifndef SELECTIONRECTWIDGET_H
#define SELECTIONRECTWIDGET_H

#include "../utils.h"
#include "../mouse/MouseAction.h"
#include "../mouse/MouseHandler.h"
#include "SelectionRectTypes.h"

constexpr int PICKER_RADIUS = 10;

class SelectionRectWidget final : public QWidget
{
    Q_OBJECT

    class SelectMouseAction final : public MouseAction
    {
        SelectionRectWidget* _sr;

        void onDown() override;
        void onMove() override;
        void onWheel(const QPoint& pixData) override;
        void onUp() override;
        void onFinish(bool canceled) override;

    public:
        explicit SelectMouseAction(SelectionRectWidget* sr) : _sr(sr)
        {
        }
    };

    class MouseHandler final : public ::MouseHandler
    {
        SelectionRectWidget* _sr;
        unique_ptr<MouseAction> createMouseAction(QMouseEvent* e) override;

    protected:
        void mouseMoveEvent(QMouseEvent* e) override;

    public:
        explicit MouseHandler(SelectionRectWidget* sr) : _sr(sr)
        {
        }
    } _mouseHandler{this};

    SelectionType _selectionType = SR_NONE;
    SelectionInfo _selection;
    QRegion _selectionRegion;
    QPoint _currentSelectionOrigin;
    QRect _currentSelection;
    QSize _imageSize;
    SelectionMode _selectionMode = Replace;

    struct // NearestState
    {
        PickerElement* element = nullptr;
        ButtonState state = None;
        // auto operator<=>(const NearestState&) const = default;
    } _nearest;

    float _scale = 1.f;
    float _scaleInv = 1.f;

    PickerElement* nearest(const QPoint& p);

public:
    explicit SelectionRectWidget(QWidget* parent);

    [[nodiscard]] SelectionType type() const { return _selectionType; }
    [[nodiscard]] bool isRemoving() const { return _selectionMode == Substract; }

    void resetDisabled();
    void resetRect(const QSize& imageSize);
    void resetRect(const QImage& sel);
    void resetPicker(const QSize& imageSize, const vector<PickerElement>& elements);

    void beginSelection(const QPoint& value);
    void updateSelection(const QPoint& value);

    void sendSelection();

    [[nodiscard]] const SelectionInfo& selection() const { return _selection; }
    [[nodiscard]] const QImage& selectionRect() const { return get<QImage>(_selection); }
    [[nodiscard]] const vector<PickerElement>& selectionElements() const { return get<vector<PickerElement>>(_selection); }

private:
    void pickNearest();
    void reset(const QSize& imageSize);
    void updateScale();
    void updateNearest(PickerElement* element);
    void updateNearest(ButtonState state);
    void updateNearest(PickerElement* element, ButtonState state);
    void updateSelectionMode(Qt::KeyboardModifiers modifiers);
    void resizeEvent(QResizeEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    bool event(QEvent* e) override;
    void updateSelectionRegion();
    void updateSelectionMode();

public slots:
    void onModifiersChange(Qt::KeyboardModifiers value);
};


#endif //SELECTIONRECTWIDGET_H
