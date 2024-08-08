//
// Created by benichn on 12/07/24.
//

#ifndef MOUSEACTION_H
#define MOUSEACTION_H

#include "../imports/qtimports.h"

class MouseHandler;

class MouseAction {
    MouseHandler *_handler;
    Qt::MouseButton _buttonDown;
    bool _hasMoved = false;
    bool _isFinished = false;
    QPoint _downPos;
    QPoint _lastPos;
    QPoint _lastDiff;

    void updateData(const QPoint &pos);
public:
    virtual ~MouseAction() = default;
    MouseAction(){}
    [[nodiscard]] Qt::MouseButton buttonDown() const { return _buttonDown; }
    [[nodiscard]] bool hasMoved() const { return _hasMoved; }
    [[nodiscard]] bool isFinished() const { return _isFinished; }
    [[nodiscard]] QPoint downPos() const { return _downPos; }
    [[nodiscard]] QPoint lastPos() const { return _lastPos; }
    [[nodiscard]] QPoint lastDiff() const { return _lastDiff; }

    void mousePressEvent(const QMouseEvent *e, MouseHandler *handler);
    void mouseReleaseEvent(const QMouseEvent *e);
    void mouseMoveEvent(const QMouseEvent *e);
    void wheelEvent(const QWheelEvent *e);

    void cancel() { finish(true); }

protected:
    void callDefault() const;
    void finish(bool canceled);

    virtual void onDown() {}
    virtual void onMove() {}
    virtual void onWheel(const QPoint &pixDelta) {}
    virtual void onUp() {}
    virtual void onFinish(bool canceled) {}

public:
    static constexpr int MINIMAL_MOVE_DISTANCE_SQ = 50;
};

#endif //MOUSEACTION_H
