//
// Created by benichn on 12/07/24.
//

#ifndef MOUSEHANDLER_H
#define MOUSEHANDLER_H

#include "../imports/stdimports.h"
#include "../imports/qtimports.h"

class MouseAction;

class MouseHandler {
    unique_ptr<MouseAction> _mouseAction;
    QEvent * _e = nullptr;
public:
    virtual ~MouseHandler() = default;
    bool event(QEvent *e);
    bool callDefault();
protected:
    virtual unique_ptr<MouseAction> createMouseAction(QMouseEvent *e) { return nullptr; }
    // events appelés en l'abscence d'une action en cours, ou si l'action les appelle explicitement
    virtual void mousePressEvent(QMouseEvent *e) {}
    virtual void mouseReleaseEvent(QMouseEvent *e) {}
    virtual void mouseDoubleClickEvent(QMouseEvent *e) {}
    virtual void mouseMoveEvent(QMouseEvent *e) {}
    virtual void wheelEvent(QWheelEvent *e) {}
};

#endif //MOUSEHANDLER_H
