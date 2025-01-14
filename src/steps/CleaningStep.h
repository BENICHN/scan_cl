//
// Created by benichn on 01/08/24.
//

#ifndef CLEANINGSTEP_H
#define CLEANINGSTEP_H
#include "Step.h"
#include "../imports/opencvimports.h"

struct CCStatsWithTimestamp
{
    stf::file_time_type lastWriteTime;
    CCStats stats;
};

class CleaningStep final : public Step {
    // vector<int> _selectedElements;
    mutable optional<CCStatsWithTimestamp> _stats;
    CCStats& stats() const;

public:
    explicit CleaningStep(int pageId);

    bool applyChoice(const SelectionInfo& selection) override;
    Task<StepSataus> run() override;
    [[nodiscard]] PreviewerSettings previewerSettings() const override;
    [[nodiscard]] string name() const override { return "Nettoyage"; }
};



#endif //CLEANINGSTEP_H
