//
// Created by benichn on 28/07/24.
//

#ifndef BOOK_H
#define BOOK_H

#include "../qtimports.h"

struct PageSettings
{
    bool flip = false;
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

enum PageType
{
    PT_BLACK,
    PT_COLOR,
    PT_GRAY
};

enum PageStep
{
    PS_NONE,
    PS_CROPPING,
    PS_MERGING,
    PS_CLEANING,
    PS_FINAL,
};

enum PageStatus
{
    PST_IDLE,
    PST_WORKING,
    PST_WAITING
};

struct LastStep
{
    PageStep step = PS_NONE;
    PageSettings settings;
    int colorAreaHash = -1;
};

struct Page
{
    int id;
    PageType type;
    string source;
    int subPage;
    PageSettings settings;
    LastStep lastStep;
    PageStatus status;
    [[nodiscard]] bool isOld() const;
};

struct Book {
    string title;
    string sourcesDir;
    string destDir;
    vector<Page> pages;
    [[nodiscard]] const Page* get(int id) const;
    [[nodiscard]] int id(int index) const;
};

#endif //BOOK_H
