//
// Created by benichn on 30/07/24.
//

#include "works.h"

#include "app.h"
#include "utils.h"
#include "jsonimports.h"

void Works::enqueue(const Work& work)
{
    waitingWorks.push(work);
    launch();
}

Task<bool> nextPageWork(const int pageId)
{
    Book* book = app()->book();
    const Page* page = book->get(pageId);
    switch (page->nextStep().value())
    {
    case PS_CROPPING:
        {
            QProcess p;
            p.start("python3", {
                        "/home/benichn/prog/cpp/scan/algo/crop.py",
                        (book->sourcesDir() + page->source).c_str(),
                        book->generatedDir().c_str(),
                        to_string(pageId).c_str(),
                        page->toJsonSettings().dump().c_str()
                    });
            co_await qCoro(p).waitForFinished(60000);
            co_return p.exitCode() == 0;
        }
    case PS_MERGING:
        {

        }
        break;
    case PS_CLEANING:
        {

        }
        break;
    case PS_FINAL:
        {

        }
        break;
    }
}

Task<> runWork(const Work& work)
{
    while (true)
    {
        Book* book = app()->book();
        if (!book->setPageWorkingIfReady(work.pageId)) co_return;
        bool result = co_await nextPageWork(work.pageId);
        if (result)
        {
            book->validatePage(work.pageId);
        }
        else
        {
            book->invalidatePage(work.pageId);
        }
    }
}

Task<> Works::launch()
{
    if (!runningWorks.empty()) co_return;
    while (true)
    {
        if (waitingWorks.empty() && runningWorks.empty()) co_return;
        if (!waitingWorks.empty() && runningWorks.size() < 5)
        {
            const Work wk = waitingWorks.front();
            waitingWorks.pop();
            runningWorks[wk.pageId] = wk;
            runWork(wk).then([this, wk]
            {
                runningWorks.erase(wk.pageId);
            });
        }
        else
        {
            co_await delay(100);
        }
    }
}
