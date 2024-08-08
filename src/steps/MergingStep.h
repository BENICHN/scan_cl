//
// Created by benichn on 01/08/24.
//

#ifndef MERGINGSTEP_H
#define MERGINGSTEP_H
#include "Step.h"


class MergingStep final : public Step
{
public:
    explicit MergingStep(int pageId);

    bool applyChoice(const SelectionInfo& selection) override;
    [[nodiscard]] PreviewerSettings previewerSettings() const override;
    Task<StepSataus> run() override;
    [[nodiscard]] bool enabled() const override;
    [[nodiscard]] string name() const override { return "Assemblage"; }
};


#endif //MERGINGSTEP_H
