//
// Created by benichn on 30/07/24.
//

#include "works.h"
#include <QtConcurrent/QtConcurrent>
#include <QCoro/QCoroProcess>

#include "app.h"
#include "utils.h"

QCoro::Task<bool> nextPageWork(const int pageId)
{
    const Book* book = App::instance()->database()->book();
    const Page* page = book->get(pageId);
    if (page->isOld() || page->lastStep.step == PS_NONE)
    {
        QProcess p;
        p.start("python3", {
                    "/home/benichn/prog/cpp/scan/rot.py",
                    (book->sourcesDir + "/" + page->source).c_str(),
                    book->destDir.c_str(),
                    to_string(pageId).c_str(),
                });
        co_await qCoro(p).waitForFinished(60000);
        int x = p.exitCode();
        qDebug() << pageId;
        co_return x == 0;
    }
    switch (page->lastStep.step)
    {
    case PS_CROPPING:
        break;
    case PS_MERGING:
        break;
    case PS_CLEANING:
        break;
    case PS_FINAL:
        break;
    }
}

void Works::enqueue(Work work)
{
    waitingWorks.push(work);
    launch();
}

QCoro::Task<> Works::launch()
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
            nextPageWork(wk.pageId).then([this, wk]
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
