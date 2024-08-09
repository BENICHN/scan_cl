//
// Created by benichn on 01/08/24.
//

#ifndef PAGESTEP_H
#define PAGESTEP_H

#include "../imports/stdimports.h"
#include "../imports/qcoroimports.h"
#include "../imports/jsonimports.h"
#include "../components/PreviewerSelectionTypes.h"

struct Page;

enum StepSataus
{
    SST_NOTRUN,
    SST_COMPLETE,
    SST_WORKING,
    SST_WAITING,
    SST_ERROR
};

struct Step
{
    StepSataus status = SST_NOTRUN;
    json settings;

protected:
    const int _pageId;
    [[nodiscard]] const Page& page() const;
    explicit Step(int pageId);

public:
    virtual ~Step() = default;
    virtual bool applyChoice(const SelectionInfo& selection) { return false; }
    virtual Task<StepSataus> run() = 0;
    [[nodiscard]] virtual bool enabled() const { return true; }
    [[nodiscard]] virtual string name() const = 0;
    [[nodiscard]] virtual PreviewerSettings previewerSettings() const { return {}; }
    [[nodiscard]] json realSettings() const;
};


#endif //PAGESTEP_H