//
// Created by benichn on 01/08/24.
//

#include "CroppingStep.h"

#include "../app.h"
#include "../imports/qtimports.h"
#include "../data/book.h"

CroppingStep::CroppingStep(const int pageId): Step(pageId)
{
}

Task<StepSataus> CroppingStep::run()
{
    auto& book = app().book();
    const auto& page = book.page(_pageId);

    json j = realSettings();
    j["subPage"] = page.subPage;
    j["colorMode"] = page.colorMode;

    QStringList argList = {
        "/home/benichn/prog/cpp/scan/algo/crop.py",
        j.dump().c_str(),
        book.pageSourceBWPath(_pageId).c_str(),
        book.generatedDir().c_str(),
        to_string(_pageId).c_str(),
    };
    if (page.colorMode != PT_BLACK)
    {
        argList.insert(3, book.pageSourceCGPath(_pageId).c_str());
    }
    QProcess p;
    p.start("python3", argList);
    co_await qCoro(p).waitForFinished(60000);
    const auto err = p.readAllStandardError().toStdString();
    if (!err.empty())
    {
        book.setPageLastError(_pageId, err);
    }
    co_return p.exitCode() == 0 ? SST_COMPLETE : SST_ERROR;
}
