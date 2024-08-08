//
// Created by benichn on 03/08/24.
//

#ifndef PREVIEWERSELECTIONTYPES_H
#define PREVIEWERSELECTIONTYPES_H

#include "../widgets/SelectionRectTypes.h"
#include "../widgets/PreviewerSelectorTypes.h"

enum class ImageOrigin
{
    SOURCE,
    GENERATED
};

struct PreviewerSettings
{
    optional<ImageOrigin> origin;
    optional<PreviewerColor> color;
    optional<SelectionType> selectionType;
};

#endif //PREVIEWERSELECTIONTYPES_H
