//
// Created by benichn on 30/07/24.
//

#include "works.h"

#include "app.h"
#include "imports/jsonimports.h"

bool Works::enqueue(const Work& work)
{
    if (isEnqueued(work.pageId)) return false;
    _waitingWorks.emplace_back(work);
    launch();
    return true;
}

bool Works::isEnqueued(int pageId)
{
    return _runningWorks.contains(pageId) || str::any_of(_waitingWorks, [=](const Work& work)
    {
        return work.pageId == pageId;
    });
}

Task<> runWork(const Work& work)
{
    Book& book = app().book();
    while (true)
    {
        const auto res = co_await book.runPage(work.pageId);
        if (!work.stopAtAsk && res == SST_WAITING)
        {
            while (true)
            {
                const auto [ id, accepted ] = co_await qCoro(&book, &Book::choiceAccepted);
                if (id == work.pageId && accepted)
                {
                    break;
                }
            }
        }
        else if (!work.allSteps || book.page(work.pageId).status() == PST_COMPLETED) break;
    }
}

Task<> Works::launch()
{
    if (!_runningWorks.empty()) co_return;
    while (true)
    {
        if (_waitingWorks.empty() && _runningWorks.empty()) co_return;
        if (!_waitingWorks.empty() && _runningWorks.size() < 5)
        {
            const Work wk = _waitingWorks.front();
            _waitingWorks.pop_front();
            _runningWorks[wk.pageId] = wk;
            runWork(wk).then([=]
            {
                _runningWorks.erase(wk.pageId);
                emit workFinished();
            });
        }
        else
        {
            co_await qCoro(this, &Works::workFinished);
        }
    }
}
