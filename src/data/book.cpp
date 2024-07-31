//
// Created by benichn on 29/07/24.
//

#include "book.h"

#include "../app.h"

void to_json(json& j, const PageSettings& v)
{
    if (v.flip.has_value()) j["flip"] = v.flip;
    if (v.finalSize.has_value()) j["finalSize"] = v.finalSize;
    if (v.maxBlockDist.has_value()) j["maxBlockDist"] = v.maxBlockDist;
    if (v.minConnectedBlockSize.has_value()) j["minConnectedBlockSize"] = v.minConnectedBlockSize;
    if (v.minBlockSize.has_value()) j["minBlockSize"] = v.minBlockSize;
    if (v.cropOverflow.has_value()) j["cropOverflow"] = v.cropOverflow;
    if (v.whiteThreshold.has_value()) j["whiteThreshold"] = v.whiteThreshold;
    if (v.smallImageBlocksArea.has_value()) j["smallImageBlocksArea"] = v.smallImageBlocksArea;
    if (v.whiteColorThreshold.has_value()) j["whiteColorThreshold"] = v.whiteColorThreshold;
    if (v.colorGamma.has_value()) j["colorGamma"] = v.colorGamma;
    if (v.maxBigCCColorMean.has_value()) j["maxBigCCColorMean"] = v.maxBigCCColorMean;
    if (v.blurSize.has_value()) j["blurSize"] = v.blurSize;
    if (v.maxBlurredCCArea.has_value()) j["maxBlurredCCArea"] = v.maxBlurredCCArea;
}

void from_json(const json& j, PageSettings& v)
{
    if (j.contains("flip")) v.flip = j["flip"];
    if (j.contains("finalSize")) v.finalSize = j["finalSize"];
    if (j.contains("maxBlockDist")) v.maxBlockDist = j["maxBlockDist"];
    if (j.contains("minConnectedBlockSize")) v.minConnectedBlockSize = j["minConnectedBlockSize"];
    if (j.contains("minBlockSize")) v.minBlockSize = j["minBlockSize"];
    if (j.contains("cropOverflow")) v.cropOverflow = j["cropOverflow"];
    if (j.contains("whiteThreshold")) v.whiteThreshold = j["whiteThreshold"];
    if (j.contains("smallImageBlocksArea")) v.smallImageBlocksArea = j["smallImageBlocksArea"];
    if (j.contains("whiteColorThreshold")) v.whiteColorThreshold = j["whiteColorThreshold"];
    if (j.contains("colorGamma")) v.colorGamma = j["colorGamma"];
    if (j.contains("maxBigCCColorMean")) v.maxBigCCColorMean = j["maxBigCCColorMean"];
    if (j.contains("blurSize")) v.blurSize = j["blurSize"];
    if (j.contains("maxBlurredCCArea")) v.maxBlurredCCArea = j["maxBlurredCCArea"];
}

optional<PageStep> stepAfter(const optional<PageStep>& step)
{
    if (!step) return PS_CROPPING;
    switch (step.value())
    {
    case PS_CROPPING:
        return PS_MERGING;
    case PS_MERGING:
        return PS_CLEANING;
    case PS_CLEANING:
        return PS_FINAL;
    case PS_FINAL:
        return nullopt;
    }
}

optional<PageStep> Page::nextStep() const
{
    return stepAfter(lastStep);
}

PageSettings Page::realSettings() const
{
    PageSettings res = app()->book()->globalSettings;
    if (settings.flip.has_value()) res.flip = settings.flip;
    if (settings.finalSize.has_value()) res.finalSize = settings.finalSize;
    if (settings.maxBlockDist.has_value()) res.maxBlockDist = settings.maxBlockDist;
    if (settings.minConnectedBlockSize.has_value()) res.minConnectedBlockSize = settings.minConnectedBlockSize;
    if (settings.minBlockSize.has_value()) res.minBlockSize = settings.minBlockSize;
    if (settings.cropOverflow.has_value()) res.cropOverflow = settings.cropOverflow;
    if (settings.whiteThreshold.has_value()) res.whiteThreshold = settings.whiteThreshold;
    if (settings.smallImageBlocksArea.has_value()) res.smallImageBlocksArea = settings.smallImageBlocksArea;
    if (settings.whiteColorThreshold.has_value()) res.whiteColorThreshold = settings.whiteColorThreshold;
    if (settings.colorGamma.has_value()) res.colorGamma = settings.colorGamma;
    if (settings.maxBigCCColorMean.has_value()) res.maxBigCCColorMean = settings.maxBigCCColorMean;
    if (settings.blurSize.has_value()) res.blurSize = settings.blurSize;
    if (settings.maxBlurredCCArea.has_value()) res.maxBlurredCCArea = settings.maxBlurredCCArea;
    return res;
}

json Page::toJsonSettings() const
{
    json j = realSettings();
    j["subPage"] = subPage;
    j["colorMode"] = colorMode;
    return j;
}

Page* Book::get(const int id)
{
    for (Page& page : pages)
    {
        if (page.id == id)
        {
            return &page;
        }
    }
    return nullptr;
}

int Book::id(const int index) const
{
    return pages[index].id;
}

string Book::sourcesDir() const
{
    return root + "/sources/";
}

string Book::outputDir() const
{
    return root + "/output/";
}

string Book::generatedDir() const
{
    return root + "/.generated/";
}

string Book::sourcesThumbnailsdDir() const
{
    return root + "/.thumbnails/sources/";
}

string Book::mergingChoicesDir() const
{
    return root + "/.choices/merging/";
}

bool Book::setPageWorkingIfReady(int id)
{
    Page* p = get(id);
    if (p->status == PST_READY)
    {
        p->status = PST_WORKING;
        emit app()->pageStatusChanged(id);
        return true;
    }
    return false;
}

void Book::invalidatePage(const int id)
{
    get(id)->status = PST_ERROR;
    std::remove((generatedDir() + to_string(id) + "*").c_str());
    emit app()->pageStatusChanged(id);
}

void Book::validatePage(const int id)
{
    Page* p = get(id);
    p->lastStep = p->nextStep();
    switch (p->lastStep.value())
    {
    case PS_CROPPING:
        p->status = p->colorMode == PT_BLACK
                        ? PST_WAITING
                        : std::filesystem::exists(mergingChoicesDir() + to_string(id))
                        ? PST_READY
                        : PST_WAITING;
        break;
    case PS_MERGING:
        p->status = PST_WAITING;
        break;
    case PS_CLEANING:
        p->status = PST_READY;
        break;
    case PS_FINAL:
        p->status = PST_COMPLETED;
        break;
    }
    emit app()->pageStatusChanged(id);
}

string Book::getPageSourceThumbnail(int id)
{
    Page* p = get(id);
    string path = sourcesThumbnailsdDir() + p->source + ".png";
    if (std::filesystem::exists(path)) // !
    {
        return path;
    }
    const auto pix = QImage((sourcesDir() + p->source).c_str());
    const QImage scaled = pix.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaled.save(path.c_str()); // !
    return path;
}
