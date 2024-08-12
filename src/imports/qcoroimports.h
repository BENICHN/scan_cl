//
// Created by benichn on 31/07/24.
//

#ifndef OTHERIMPORTS_H
#define OTHERIMPORTS_H

#include <QCoro/QCoroTask>
#include <QCoro/QCoroFuture>
#include <QCoro/QCoroProcess>
#include <QCoro/QCoroSignal>
#include <QCoro/QCoroTimer>
#include <QtConcurrent/qtconcurrentrun.h>

using QCoro::Task;

class CancelToken
{
    bool _isCanceled = false;

public:
    [[nodiscard]] bool isCanceled() const { return _isCanceled; }
    void cancel() { _isCanceled = true; }
};

#endif //OTHERIMPORTS_H
