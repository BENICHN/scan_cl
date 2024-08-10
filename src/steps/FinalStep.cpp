//
// Created by benichn on 01/08/24.
//

#include "FinalStep.h"
#include "../app.h"

FinalStep::FinalStep(const int pageId): Step(pageId)
{
}

constexpr int cmToPix(const float cm, const float dpi)
{
    return static_cast<int>(cm * dpi / 2.54);
}

constexpr int cmToPix(const float cm, const float forced, const float dpi)
{
    const float x = forced < 0 ? cm : forced;
    return static_cast<int>(x * dpi / 2.54);
}

QMargins FinalStep::margins(const int w, const int h) const
{
    const json j = realSettings();
    int t, l, b, r;
    const float dpi = j.at("dpi");
    const int cropOverflow = app().book().page(_pageId).croppingStep().realSettings().at("cropOverflow");
    const int W = cmToPix(j.at("finalWidthCm"), dpi);
    const int H = cmToPix(j.at("finalHeightCm"), dpi);
    const auto alignmentH = j.at("alignmentH").get_ref<const string&>()[0];
    const auto alignmentV = j.at("alignmentV").get_ref<const string&>()[0];

    switch (alignmentH)
    {
    case 'l':
        l = cmToPix(j.at("leftMarginCm"), j.at("forcedLeftCm"), dpi) - cropOverflow;
        r = W - (l + w);
        break;
    case 'r':
        r = cmToPix(j.at("rightMarginCm"), j.at("forcedRightCm"), dpi) - cropOverflow;
        l = W - (r + w);
        break;
    case 'c':
        l = (W - w) / 2;
        r = W - (l + w);
        break;
    default:
        throw runtime_error("invalid alignmentH");
    }
    switch (alignmentV)
    {
    case 't':
        t = cmToPix(j.at("topMarginCm"), j.at("forcedTopCm"), dpi) - cropOverflow;
        b = H - (t + h);
        break;
    case 'b':
        b = cmToPix(j.at("bottomMarginCm"), j.at("forcedBottomCm"), dpi) - cropOverflow;
        t = H - (b + h);
        break;
    case 'c':
        t = (H - h) / 2;
        b = H - (t + h);
        break;
    case 'h':
        t = cmToPix(j.at("headerHeightCm"), dpi) - cropOverflow;
        b = H - (t + h);
        break;
    default:
        throw runtime_error("invalid alignmentV");
    }
    return {l, t, r, b};
}

Task<StepSataus> FinalStep::run()
{
    co_return co_await QtConcurrent::run([=]
    {
        try
        {
            const auto& book = app().book();
            const auto bw = imread(book.pageGeneratedBWPath(_pageId), cv::IMREAD_GRAYSCALE);
            const auto finalMargins = margins(bw.cols, bw.rows);
            const auto finalBw = surroundWith(bw, finalMargins.top(), finalMargins.left(), finalMargins.bottom(),
                                              finalMargins.right(), 255);
            imwrite(book.pageGeneratedBWPath(_pageId), finalBw);
            const auto colorMode = book.page(_pageId).colorMode;
            if (colorMode != PT_BLACK)
            {
                const bool color = colorMode == PT_COLOR;
                const auto cg = imread(book.pageGeneratedCGPath(_pageId),
                                       color ? cv::IMREAD_COLOR : cv::IMREAD_GRAYSCALE);
                const auto finalCg = surroundWith(cg, finalMargins.top(), finalMargins.left(), finalMargins.bottom(),
                                                  finalMargins.right(), color ? Scalar{255, 255, 255} : 255);
                imwrite(book.pageGeneratedCGPath(_pageId), finalCg);
            }
            return SST_COMPLETE;
        }
        catch (...)
        {
            return SST_ERROR;
        }
    });
}
