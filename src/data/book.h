//
// Created by benichn on 28/07/24.
//

#ifndef BOOK_H
#define BOOK_H

#include "../qtimports.h"

struct BasicPageSettings
{
    bool flip = false;
    optional<QSize> finalSize;
};

struct OtherPageSettings
{
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
};

enum PageType
{
    BLACK,
    COLOR,
    GRAY
};

enum PageStatus
{
    IDLE,
    WAITING,
    COMPLETED
};

struct Page
{
    PageType type;
    PageStatus status;
    string source;
    int subPage;
    BasicPageSettings basicSettings;
    OtherPageSettings otherSettings;
    string dest;
};

struct Book {
    string title;
    string sourcesDir;
    string destDir;
    vector<Page> pages;
};

#endif //BOOK_H
