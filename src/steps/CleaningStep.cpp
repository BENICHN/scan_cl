//
// Created by benichn on 01/08/24.
//

#include "CleaningStep.h"

#include "../app.h"
#include "../imports/opencvimports.h"

CCStats& CleaningStep::stats() const
{
    const auto path = app().book().pageGeneratedBigsMaskPath(_pageId);
    const auto t = stf::last_write_time(path);
    if (!_stats.has_value() || _stats.value().lastWriteTime != t)
    {
        _stats = {
            t,
            connectedComponentsWithStats(imread(path, cv::IMREAD_GRAYSCALE))
        };
    }
    return _stats.value().stats;
}

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
        co_return co_await QtConcurrent::run([&book, this]
        {
            try
            {
                auto bw = imread(book.pageGeneratedBWPath(_pageId), cv::IMREAD_GRAYSCALE);
                const auto& st = stats();
                auto sel = book.pageChosenBigs(_pageId);
                for (auto& i : sel) { ++i; }
                const auto mask = isIn(st.labels, sel);
                bw.setTo(255, mask);
                imwrite(book.pageGeneratedBWPath(_pageId), bw);
                stf::remove(book.pageGeneratedBigsMaskPath(_pageId));
                return SST_COMPLETE;
            }
            catch (...)
            {
                return SST_ERROR;
            }
        });
    }
    co_return SST_WAITING;
}

PreviewerSettings CleaningStep::previewerSettings() const
{
    switch (status)
    {
    case SST_WAITING:
        {
            return {
                ImageOrigin::GENERATED,
                PWC_BW,
                SR_PICKER,
                pickerElementsFromStats(stats(), true)
            };
        }
    default:
        return {};
    }
}
