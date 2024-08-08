//
// Created by benichn on 01/08/24.
//

#include "MergingStep.h"

#include "../app.h"
#include <magic_enum.hpp>

MergingStep::MergingStep(const int pageId): Step(pageId)
{
}

bool MergingStep::applyChoice(const SelectionInfo& selection)
{
    return app().book().chooseMergingMask(_pageId, get<QImage>(selection));
}

PreviewerSettings MergingStep::previewerSettings() const
{
    switch (status)
    {
    case SST_WAITING:
        return {
            ImageOrigin::GENERATED,
            PWC_CG,
            SR_RECT
        };
    default:
        return {};
    }
}

Task<StepSataus> MergingStep::run()
{
    const auto& book = app().book();
    if (book.pageMergingMaskAvailable(_pageId))
    {
        QProcess p;
        p.start("python3", {
                    "/home/benichn/prog/cpp/scan/algo/amerge.py",
                    book.generatedDir().c_str(),
                    to_string(_pageId).c_str(),
                     magic_enum::enum_name(book.page(_pageId).colorMode).data(),
                    book.pageMergingMaskPath(_pageId).c_str()
                });
        co_await qCoro(p).waitForFinished(60000);
        co_return p.exitCode() == 0 ? SST_COMPLETE : SST_ERROR;
    }
    co_return SST_WAITING;
}

bool MergingStep::enabled() const
{
    return page().colorMode != PT_BLACK;
}
