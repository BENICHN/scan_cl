//
// Created by benichn on 30/07/24.
//

#ifndef WORKS_H
#define WORKS_H

#include <QCoro/QCoroFuture>
#include "stdimports.h"

struct Work
{
    int pageId = -1;
    bool allSteps = true;
    bool stopAtAsk = false;
};

class Works {
    queue<Work> waitingWorks;
    unordered_map<int, Work> runningWorks;
public:
    void enqueue(Work work);
private:
    QCoro::Task<> launch();
};



#endif //WORKS_H
