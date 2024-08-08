//
// Created by benichn on 01/08/24.
//

#include "CleaningStep.h"

#include "../app.h"

CleaningStep::CleaningStep(const int pageId): Step(pageId)
{
}

bool CleaningStep::applyChoice(const SelectionInfo& selection)
{
    return app().book().choosePageBigs(_pageId, get<vector<PickerElement>>(selection));
}

Task<StepSataus> CleaningStep::run()
{
    const auto& book = app().book();
    if (book.pageChosenBigsAvailable(_pageId))
    {
        QProcess p;
        p.start("python3", {
                    "/home/benichn/prog/cpp/scan/algo/clean.py",
                    book.generatedDir().c_str(),
                    to_string(_pageId).c_str()
                });
        co_await qCoro(p).waitForFinished(60000);
        co_return p.exitCode() == 0 ? SST_COMPLETE : SST_ERROR;
    }
    co_return SST_ERROR;
}

PreviewerSettings CleaningStep::previewerSettings() const
{
    switch (status)
    {
    case SST_WAITING:
        return {
            ImageOrigin::GENERATED,
            PWC_BW,
            SR_PICKER
        };
    default:
        return {};
    }
}
