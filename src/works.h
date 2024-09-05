//
// Created by benichn on 30/07/24.
//

#ifndef WORKS_H
#define WORKS_H

#include "imports/stdimports.h"
#include "imports/qcoroimports.h"

struct Work
{
    int pageId = -1;
    bool allSteps = true;
    bool stopAtAsk = false;

    [[nodiscard]] bool canceled() const { return _canceled; }
    void cancel()
    {
        _canceled = true;
    }

    Work() = default;
    Work(int page_id, bool all_steps, bool stop_at_ask);

private:
    bool _canceled = false;
};

class Works final : public QObject
{
    Q_OBJECT
    deque<Work> _waitingWorks;
    unordered_map<int, Work*> _runningWorks;

public:
    bool enqueue(const Work& work);
    bool isEnqueued(int pageId) const;
    void cancel(int pageId);

private:
    Task<> launch();
signals:
    void workFinished();
};


#endif //WORKS_H
