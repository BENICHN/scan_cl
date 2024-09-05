//
// Created by benichn on 30/07/24.
//

#include "works.h"

#include "app.h"
#include "imports/jsonimports.h"

Work::Work(int page_id, bool all_steps, bool stop_at_ask): pageId(page_id),
                                                           allSteps(all_steps),
                                                           stopAtAsk(stop_at_ask)
{
}

bool Works::enqueue(const Work& work)
{
    if (isEnqueued(work.pageId)) return false;
    _waitingWorks.emplace_back(work);
    launch();
    return true;
}

bool Works::isEnqueued(const int pageId) const
{
    return _runningWorks.contains(pageId) || str::any_of(_waitingWorks, [=](const auto& work)
    {
        return work.pageId == pageId;
    });
}

void Works::cancel(int pageId)
{
    const auto itR = _runningWorks.find(pageId);
    if (itR != _runningWorks.end())
    {
        itR->second->cancel();
        _runningWorks.erase(pageId);
    }
    else
    {
        erase_if(_waitingWorks, [=](const auto& wk) { return wk.pageId == pageId; });
    }
}

Task<> runWork(const Work& work)
{
    Book& book = app().book();
    while (true)
    {
        const auto res = co_await book.runPage(work);
        if (work.canceled()) co_return;
        if (!work.stopAtAsk && res == SST_WAITING)
        {
            while (true)
            {
                const auto [id, accepted] = co_await qCoro(&book, &Book::choiceAccepted);
                if (work.canceled()) co_return;
                if (id == work.pageId && accepted)
                {
                    break;
                }
            }
        }
        else if (!work.allSteps || res == SST_ERROR || res == SST_WAITING || book.page(work.pageId).status() ==
            PST_COMPLETED) break;
    }
}

Task<> Works::launch()
{
    if (_waitingWorks.size() > 1) co_return;
    while (true)
    {
        if (_waitingWorks.empty() && _runningWorks.empty()) co_return;
        if (!_waitingWorks.empty() && _runningWorks.size() < 5)
        {
            const auto wk = new Work(_waitingWorks.front());
            _waitingWorks.pop_front();
            _runningWorks[wk->pageId] = wk;
            runWork(*wk).then([=]
            {
                if (!wk->canceled()) _runningWorks.erase(wk->pageId);
                delete wk;
                emit workFinished();
            });
        }
        else
        {
            co_await qCoro(this, &Works::workFinished);
        }
    }
}
