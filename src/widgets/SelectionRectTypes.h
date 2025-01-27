//
// Created by benichn on 03/08/24.
//

#ifndef SELECTIONRECTTYPES_H
#define SELECTIONRECTTYPES_H

#include "../imports/opencvimports.h"
#include "../imports/stdimports.h"
#include "../imports/qtimports.h"

enum ButtonState
{
    None,
    Hovered,
    Pressed
};

enum SelectionType
{
    SR_NONE,
    SR_RECT,
    SR_PICKER
};

enum SelectionMode
{
    Replace,
    Add,
    Substract
};

struct PickerElement
{
    QPoint centroid;
    QRect regionBounds;
    QRegion region;
    bool selected;

    static PickerElement fromStats(int x, int y, int w, int h,
                                   int cx, int cy, bool selected);
};

vector<PickerElement> pickerElementsFromStats(const CCStats& st, bool selected);

using SelectionInfo = variant<QImage, vector<PickerElement>>;

struct SelectionRectSettings
{
    SelectionType type = SR_NONE;
    optional<SelectionInfo> selection;
};

#endif //SELECTIONRECTTYPES_H
