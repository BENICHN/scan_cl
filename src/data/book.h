//
// Created by benichn on 28/07/24.
//

#ifndef BOOK_H
#define BOOK_H

#include <utility>

#include "../imports/qtimports.h"
#include "../imports/jsonimports.h"

#include "../steps/Step.h"
#include "../steps/CroppingStep.h"
#include "../steps/MergingStep.h"
#include "../steps/CleaningStep.h"
#include "../steps/FinalStep.h"

enum PageColorMode
{
    PT_BLACK,
    PT_COLOR,
    PT_GRAY
};

NLOHMANN_JSON_SERIALIZE_ENUM(PageColorMode, {
                             {PT_BLACK, "PT_BLACK"},
                             {PT_COLOR, "PT_COLOR"},
                             {PT_GRAY, "PT_GRAY"},
                             })

enum PageStatus
{
    PST_READY,
    PST_WORKING,
    PST_COMPLETED,
    PST_WAITING,
    PST_ERROR,
};

NLOHMANN_JSON_SERIALIZE_ENUM(PageStatus, {
                             {PST_READY, "PST_READY"},
                             {PST_WORKING, "PST_WORKING"},
                             {PST_COMPLETED, "PST_COMPLETED"},
                             {PST_WAITING, "PST_WAITING"},
                             {PST_ERROR, "PST_ERROR"},
                             })

struct Page
{
    int id = 0;
    PageColorMode colorMode = PT_BLACK;
    string source;
    optional<string> cgSource;
    int subPage = 1;
    vector<unique_ptr<Step>> steps;

    Page(int id, PageColorMode color_mode, string source, const optional<string>& cg_source, const int sub_page);

    // ~Page();

    Page(const Page& other) = delete;

    Page(Page&& other) noexcept
        : id(other.id),
          colorMode(other.colorMode),
          source(std::move(other.source)),
          cgSource(std::move(other.cgSource)),
          subPage(other.subPage),
          steps(std::move(other.steps))
    {
    }

    Page& operator=(const Page& other) = delete;

    Page& operator=(Page&& other) noexcept
    {
        if (this == &other)
            return *this;
        id = other.id;
        colorMode = other.colorMode;
        source = std::move(other.source);
        cgSource = std::move(other.cgSource);
        subPage = other.subPage;
        steps = std::move(other.steps);
        return *this;
    }

    //template <class Self> [[nodiscard]] auto nextStep(this Self& self)
    // [[nodiscard]] auto enabledSteps() const
    // {
    //     return steps | std::views::filter([](const auto& step)
    //     {
    //         return step->enabled();
    //     });
    // }
    [[nodiscard]] const Step* nextStep() const;
    [[nodiscard]] Step* nextStep() { return const_cast<Step*>(static_cast<const Page*>(this)->nextStep()); }
    // void setNextStepStatus(StepSataus status);

    [[nodiscard]] PreviewerSettings defaultPreviewerSettings() const;
    [[nodiscard]] PageStatus status() const;
    [[nodiscard]] const CroppingStep& croppingStep() const;
    [[nodiscard]] const MergingStep& mergingStep() const;
    [[nodiscard]] const CleaningStep& cleaningStep() const;
    [[nodiscard]] const FinalStep& finalStep() const;
    // NLOHMANN_DEFINE_TYPE_INTRUSIVE(Page, id, colorMode, source, colorSource, subPage, status)
};

class Book final : public QObject
{
    Q_OBJECT
    string _root;
    string _title;
    unordered_map<string, json> _globalSettings;
    unordered_map<int, Page> _pages;
    vector<int> _ids;

public:
    Book(string root, string title,
         const unordered_map<string, json>& global_settings) // !
    ;

    [[nodiscard]] auto& root() const { return _root; }
    void setRoot(const string& root) { _root = root; } // !
    [[nodiscard]] auto& title() const { return _title; }
    void setTitle(const string& title) { _title = title; } // !

    [[nodiscard]] auto& page(int id) const { return _pages.at(id); }
    [[nodiscard]] auto& pageAt(int index) const { return page(_ids[index]); }
    [[nodiscard]] auto& ids() const { return _ids; }
    [[nodiscard]] auto& pages() const { return _pages; }
    [[nodiscard]] json globalSettings(const string& name) const;

    // dirs
    string sourcesDir() const;
    string outputDir() const;
    string generatedDir() const;
    string sourcesThumbnailsdDir() const;
    string mergingChoicesDir() const;
    string cleaningChoicesDir() const;

    // generated images
    string pageSourceBWPath(int id) const;
    string pageSourceCGPath(int id) const;
    string pageGeneratedBWPath(int id) const;
    string pageGeneratedCGPath(int id) const;
    bool pageHasGenerated(int id) const;

    // merging mask
    string pageMergingMaskPath(int id) const;
    bool pageMergingMaskAvailable(int id) const;
    bool chooseMergingMask(int id, const QImage& mask) const;
    // mix
    bool pageMixedAvailable(int id) const;
    QPixmap pageGeneratedMixPixmap(int id) const;

    // cleaning generated
    string pageGeneratedBigsMaskPath(int id) const;
    // chosen
    string pageChosenBigsPath(int id) const;
    bool pageChosenBigsAvailable(int id) const;
    vector<int> pageChosenBigs(int id) const;
    bool choosePageBigs(int id, const vector<PickerElement>& elements) const;

    // thumbnails
    string getPageSourceThumbnail(int id) const;

    // mutations
    void applyChoiceToPage(int id, const SelectionInfo& selection);
    Task<StepSataus> runPage(int id);
    bool insertPage(int index, Page&& page);
    bool insertPageFront(Page&& page);
    bool insertPageBack(Page&& page);
signals:
    void choiceAccepted(int pageId, bool accepted);
    void pageStatusChanged(int pageId);
};

#endif //BOOK_H
