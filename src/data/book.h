//
// Created by benichn on 28/07/24.
//

#ifndef BOOK_H
#define BOOK_H

#include "../qtimports.h"
#include "../jsonimports.h"

enum PageColorMode
{
    PT_BLACK,
    PT_COLOR,
    PT_GRAY
};

NLOHMANN_JSON_SERIALIZE_ENUM(PageColorMode, {
                             {PT_BLACK, "PT_BLACK"},
                             {PT_COLOR, "PT_COLOR"},
                             {PT_GRAY, "PT_GRAY"},
                             })

struct PageSettings
{
    optional<bool> flip;
    optional<QSize> finalSize;
    optional<int> maxBlockDist;
    optional<int> minConnectedBlockSize;
    optional<int> minBlockSize;
    optional<int> cropOverflow;
    optional<int> whiteThreshold;
    optional<int> smallImageBlocksArea;
    optional<int> whiteColorThreshold;
    optional<float> colorGamma;
    optional<int> maxBigCCColorMean;
    optional<QSize> blurSize;
    optional<int> maxBlurredCCArea;
    bool operator==(const PageSettings& settings) const = default;
};

void to_json(json& j, const PageSettings& v);
void from_json(const json& j, PageSettings& v);

enum PageStep
{
    PS_CROPPING,
    PS_MERGING,
    PS_CLEANING,
    PS_FINAL,
};

NLOHMANN_JSON_SERIALIZE_ENUM(PageStep, {
                             {PS_CROPPING, "PS_CROPPING"},
                             {PS_MERGING, "PS_MERGING"},
                             {PS_CLEANING, "PS_CLEANING"},
                             {PS_FINAL, "PS_FINAL"},
                             })

PageStep stepAfter(PageStep step);

enum PageStatus
{
    PST_READY,
    PST_WORKING,
    PST_COMPLETED,
    PST_WAITING,
    PST_ERROR,
};

NLOHMANN_JSON_SERIALIZE_ENUM(PageStatus, {
                             {PST_READY, "PST_READY"},
                             {PST_WORKING, "PST_WORKING"},
                             {PST_COMPLETED, "PST_COMPLETED"},
                             {PST_WAITING, "PST_WAITING"},
                             {PST_ERROR, "PST_ERROR"},
                             })

struct Page
{
    int id = 0;
    PageColorMode colorMode = PT_BLACK;
    string source;
    int subPage = 1;
    PageSettings settings;
    optional<PageStep> lastStep;
    PageStatus status = PST_READY;
    optional<PageStep> nextStep() const;
    PageSettings realSettings() const;
    json toJsonSettings() const;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Page, id, colorMode, source, subPage, settings, lastStep, status)
};

struct Book
{
    string root;
    string title;
    PageSettings globalSettings;
    vector<Page> pages;
    [[nodiscard]] Page* get(int id);
    [[nodiscard]] int id(int index) const;
    string sourcesDir() const;
    string outputDir() const;
    string generatedDir() const;
    string sourcesThumbnailsdDir() const;
    string mergingChoicesDir() const;
    bool setPageWorkingIfReady(int id);
    void invalidatePage(int id);
    void validatePage(int id);
    string getPageSourceThumbnail(int id);
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Book, title, pages)
};

#endif //BOOK_H
