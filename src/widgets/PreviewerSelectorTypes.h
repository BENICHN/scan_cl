//
// Created by benichn on 03/08/24.
//

#ifndef PREVIEWERSELECTORTYPES_H
#define PREVIEWERSELECTORTYPES_H

enum PreviewerMode
{
    PWM_SRC,
    PWM_ASK,
    PWM_RES
};

enum PreviewerColor
{
    PWC_BW,
    PWC_CG,
    PWC_MIX,
};

struct PreviewerSelection
{
    PreviewerMode mode;
    PreviewerColor color;
    bool operator==(const PreviewerSelection& previewer_selection) const = default;
};

#endif //PREVIEWERSELECTORTYPES_H
