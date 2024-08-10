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
        co_return co_await QtConcurrent::run([&book, this]
        {
            try
            {
                const bool color = book.page(_pageId).colorMode == PT_COLOR;
                const auto mask = imread(book.pageMergingMaskPath(_pageId), cv::IMREAD_GRAYSCALE);
                auto bigs = imread(book.pageGeneratedBigsMaskPath(_pageId), cv::IMREAD_GRAYSCALE);
                auto bw = imread(book.pageGeneratedBWPath(_pageId), cv::IMREAD_GRAYSCALE);
                auto cg = imread(book.pageGeneratedCGPath(_pageId), color ? cv::IMREAD_COLOR : cv::IMREAD_GRAYSCALE);
                bigs.setTo(0, mask);
                bw.setTo(255, mask);
                cg.setTo(color ? Scalar{255, 255, 255} : 255, ~mask);
                imwrite(book.pageGeneratedBigsMaskPath(_pageId), bigs);
                imwrite(book.pageGeneratedBWPath(_pageId), bw);
                imwrite(book.pageGeneratedCGPath(_pageId), cg);
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

bool MergingStep::enabled() const
{
    return page().colorMode != PT_BLACK;
}
