//
// Created by benichn on 01/08/24.
//

#ifndef FINALSTEP_H
#define FINALSTEP_H
#include "Step.h"

class FinalStep final : public Step
{
public:
    explicit FinalStep(int pageId);

    [[nodiscard]] QMargins margins(int w, int h) const;
    Task<StepSataus> run() override;
    [[nodiscard]] string name() const override { return "Finalisation"; }
};

#endif //FINALSTEP_H
