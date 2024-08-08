//
// Created by benichn on 01/08/24.
//

#ifndef CROPPINGSTEP_H
#define CROPPINGSTEP_H

#include "Step.h"
#include "../imports/jsonimports.h"

class CroppingStep final : public Step
{
    // optional<bool> flip;
    // optional<int> maxBlockDist;
    // optional<int> minConnectedBlockSize;
    // optional<int> minBlockSize;
    // optional<int> cropOverflow;
    // optional<int> whiteThreshold;
    // optional<int> smallImageBlocksArea;
    // optional<int> whiteColorThreshold;
    // optional<float> colorGamma;
    // optional<int> maxBigCCColorMean;
    // optional<QSize> blurSize;
    // optional<int> maxBlurredCCArea;

public:
    explicit CroppingStep(int pageId);

    Task<StepSataus> run() override;
    [[nodiscard]] string name() const override { return "Recadrage"; }
};

#endif //CROPPINGSTEP_H
