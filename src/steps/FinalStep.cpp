//
// Created by benichn on 01/08/24.
//

#include "FinalStep.h"
#include "../app.h"

FinalStep::FinalStep(const int pageId): Step(pageId)
{
}

Task<StepSataus> FinalStep::run()
{
    const auto& book = app().book();
    const json j = realSettings();
    const QStringList argList = {
        "/home/benichn/prog/cpp/scan/algo/final.py",
        j.dump().c_str(),
        book.generatedDir().c_str(),
        to_string(_pageId).c_str(),
    };
    QProcess p;
    p.start("python3", argList);
    co_await qCoro(p).waitForFinished(60000);
    co_return p.exitCode() == 0 ? SST_COMPLETE : SST_ERROR;
}
