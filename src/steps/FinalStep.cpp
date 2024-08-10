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
    const float dpi = j["dpi"];
    const int cropOverflow = app().book().page(_pageId).croppingStep().realSettings()["cropOverflow"];
    const int W = cmToPix(j["finalWidthCm"], dpi);
    const int H = cmToPix(j["finalHeightCm"], dpi);
    const auto alignmentH = j["alignmentH"].get<char>();
    const auto alignmentV = j["alignmentV"].get<char>();

    switch (alignmentH)
    {
    case 'l':
        l = cmToPix(j["leftMarginCm"], j["forcedLeftCm"], dpi) - cropOverflow;
        r = W - (l + w);
        break;
    case 'r':
        r = cmToPix(j["rightMarginCm"], j["forcedRightCm"], dpi) - cropOverflow;
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
        t = cmToPix(j["topMarginCm"], j["forcedTopCm"], dpi) - cropOverflow;
        b = H - (t + h);
        break;
    case 'b':
        b = cmToPix(j["bottomMarginCm"], j["forcedBottomCm"], dpi) - cropOverflow;
        t = H - (b + h);
        break;
    case 'c':
        t = (H - h) / 2;
        b = H - (t + h);
        break;
    case 'h':
        t = cmToPix(j["headerHeightCm"], dpi) - cropOverflow;
        b = H - (t + h);
        break;
    default:
        throw runtime_error("invalid alignmentV");
    }
    return {l, t, r, b};
}

Task<StepSataus> FinalStep::run()
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
        const auto cg = imread(book.pageGeneratedCGPath(_pageId), color ? cv::IMREAD_COLOR : cv::IMREAD_GRAYSCALE);
        const auto finalCg = surroundWith(cg, finalMargins.top(), finalMargins.left(), finalMargins.bottom(),
                                          finalMargins.right(), color ? Scalar{255, 255, 255} : 255);
        imwrite(book.pageGeneratedCGPath(_pageId), finalCg);
    }
    co_return SST_COMPLETE;
}
