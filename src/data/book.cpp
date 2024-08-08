//
// Created by benichn on 29/07/24.
//

#include "book.h"

#include "../app.h"
#include "../utils.h"

// const Step* Page::nextStep() const
// {
//     for (const Step* s : steps)
//     {
//         if (s->status() != SST_COMPLETE) return s;
//     }
//     return nullptr;
// }

Page::Page(const int id, PageColorMode color_mode, string source, const optional<string>& cg_source, const int sub_page): id(id),
    colorMode(color_mode),
    source(std::move(source)),
    cgSource(cg_source),
    subPage(sub_page)
{
    steps.emplace_back(make_unique<CroppingStep>(CroppingStep(id)));
    steps.emplace_back(make_unique<MergingStep>(MergingStep(id))); // !
    steps.emplace_back(make_unique<CleaningStep>(CleaningStep(id)));
    steps.emplace_back(make_unique<FinalStep>(FinalStep(id)));
}

// Page::~Page()
// {
//     qDebug() << this << " deleted !";
//     for (auto* step : steps)
//     {
//         delete step;
//     }
// }

const Step* Page::nextStep() const
{
    for (const auto& s : steps)
    {
        if (s->enabled() && s->status != SST_COMPLETE) return s.get();
    }
    return nullptr;
}

PreviewerSettings Page::defaultPreviewerSettings() const
{
    auto* step = nextStep();
    auto astgs = step ? step->previewerSettings() : PreviewerSettings{};
    const auto& book = app().book();
    if (!astgs.origin.has_value())
    {
        astgs.origin = book.pageHasGenerated(id) ? ImageOrigin::GENERATED : ImageOrigin::SOURCE;
    }
    if (!astgs.color.has_value())
    {
        if (colorMode == PT_BLACK)
        {
            astgs.color = PWC_BW;
        }
        else
        {
            astgs.color = mergingStep().status == SST_COMPLETE ? PWC_MIX : PWC_BW;
        }
    }
    if (!astgs.selectionType.has_value())
    {
        astgs.selectionType = SR_NONE;
    }
    return astgs;
}

// void Page::setNextStepStatus(StepSataus status)
// {
//     auto* step = nextStep();
//     step->status = status;
// }

PageStatus Page::status() const
{
    for (const auto& s : steps)
    {
        switch (s->status)
        {
        case SST_NOTRUN:
            return PST_READY;
        case SST_COMPLETE:
            break;
        case SST_WORKING:
            return PST_WORKING;
        case SST_WAITING:
            return PST_WAITING;
        case SST_ERROR:
            return PST_ERROR;
        }
    }
    return PST_COMPLETED;
}

const CroppingStep& Page::croppingStep() const
{
    return *static_cast<CroppingStep*>(steps.at(0).get());
}

const MergingStep& Page::mergingStep() const
{
    return *static_cast<MergingStep*>(steps.at(1).get());
}

const CleaningStep& Page::cleaningStep() const
{
    return *static_cast<CleaningStep*>(steps.at(2).get());
}

const FinalStep& Page::finalStep() const
{
    return *static_cast<FinalStep*>(steps.at(3).get());
}

// const const Page& Book::page(const int id) const
// {
//     for (const Page& page : pages)
//     {
//         if (page.id == id)
//         {
//             return &page;
//         }
//     }
//     return nullptr;
// }

// int Book::id(const int index) const
// {
//     return pages[index].id;
// }

Task<StepSataus> Book::runPage(const int id)
{
    auto& p = _pages.at(id);
    if (auto* nextStep = p.nextStep())
    {
        if (nextStep->status == SST_WORKING) co_return nextStep->status;
        nextStep->status = SST_WORKING;
        emit pageStatusChanged(id);
        const auto res = co_await nextStep->run();
        nextStep->status = res;
        emit pageStatusChanged(id);
        co_return res;
    }
    co_return SST_COMPLETE;
}

bool Book::insertPage(int index, Page&& page)
{
    const int pageId = page.id;
    if (_pages.contains(pageId)) return false;
    _pages.emplace(pageId, std::move(page));
    _ids.insert(_ids.begin() + index, pageId);
    return true;
}

bool Book::insertPageFront(Page&& page)
{
    return insertPage(0, std::move(page));
}

bool Book::insertPageBack(Page&& page)
{
    return insertPage(_pages.size(), std::move(page));
}

Book::Book(string root, string title, const unordered_map<string, json>& global_settings): _root(std::move(root)),
    _title(std::move(title)),
    _globalSettings(global_settings)
{
    insertPageBack(Page{10, PT_GRAY, "test.png", "test.png", 1});
    insertPageBack(Page{20, PT_GRAY, "test.png", "test.png", 1});
    insertPageBack(Page{30, PT_GRAY, "test.png", "test.png", 1});
    insertPageBack(Page{40, PT_GRAY, "test.png", "test.png", 1});
    insertPageBack(Page{50, PT_BLACK, "test.png", nullopt, 1});
    insertPageBack(Page{60, PT_BLACK, "test.png", nullopt, 1});
    insertPageBack(Page{70, PT_BLACK, "test.png", nullopt, 1});
    insertPageBack(Page{80, PT_BLACK, "test.png", nullopt, 1});
}

json Book::globalSettings(const string& name) const
{
    return mapAtDef(_globalSettings, name, {});
}

string Book::sourcesDir() const
{
    return _root + "/sources/";
}

string Book::outputDir() const
{
    return _root + "/output/";
}

string Book::generatedDir() const
{
    return _root + "/.generated/";
}

string Book::sourcesThumbnailsdDir() const
{
    return _root + "/.thumbnails/sources/";
}

string Book::mergingChoicesDir() const
{
    return _root + "/.choices/merging/";
}

string Book::cleaningChoicesDir() const
{
    return _root + "/.choices/cleaning/";
}

string Book::pageSourceBWPath(const int id) const
{
    return sourcesDir() + page(id).source;
}

bool Book::pageHasGenerated(const int id) const
{
    return std::filesystem::exists(pageGeneratedBWPath(id)); // !
}

string Book::pageMergingMaskPath(const int id) const
{
    return mergingChoicesDir() + to_string(id) + ".pbm";
}

bool Book::pageMergingMaskAvailable(const int id) const
{
    return std::filesystem::exists(pageMergingMaskPath(id));
}

bool Book::chooseMergingMask(const int id, const QImage& mask) const
{
    return mask.save(pageMergingMaskPath(id).c_str());
}

bool Book::pageMixedAvailable(const int id) const
{
    return page(id).mergingStep().status == SST_COMPLETE;
}

QImage Book::pageGeneratedMixImage(const int id) const
{
    // QImage mask = pageMergingMask(id);
    // QImage bw = pageGeneratedBWImage(id);
    // QImage cg = pageGeneratedCGImage(id);
    // return bw; // !
}

string Book::pageGeneratedBigsMaskPath(const int id) const
{
    return generatedDir() + to_string(id) + "_bigs.pbm";
}

vector<PickerElement> Book::pageGeneratedBigs(const int id) const
{
    // !
}

string Book::pageChosenBigsPath(const int id) const
{
    return cleaningChoicesDir() + to_string(id) + ".txt";
}

bool Book::pageChosenBigsAvailable(const int id) const
{
    const auto path = pageChosenBigsPath(id);
    if (!std::filesystem::exists(path) || !pageHasGenerated(id)) return false;
    ifstream file(path);
    string hash;
    if (!(file >> hash)) return false;
    auto realHash = calculateXXH3_64(pageGeneratedBigsMaskPath(id));
    return hash == realHash;
}

vector<int> Book::pageChosenBigs(const int id) const
{
    ifstream file(pageChosenBigsPath(id));
    vector<int> res;
    ignoreUntil(file, '\n');
    int i;
    while (file >> i)
    {
        res.emplace_back(i);
    }
    return res;
}

bool Book::choosePageBigs(const int id, const vector<PickerElement>& elements) const
{
    ofstream file(pageChosenBigsPath(id));
    file << calculateXXH3_64(pageGeneratedBigsMaskPath(id)) << '\n';
    for (int i = 0; i < elements.size(); ++i)
    {
        if (elements[0].selected) file << i << ' ';
    }
    return true; // !
}

string Book::pageSourceCGPath(const int id) const
{
    return sourcesDir() + page(id).cgSource.value();
}

string Book::pageGeneratedBWPath(const int id) const
{
    return generatedDir() + to_string(id) + ".pbm";
}

string Book::pageGeneratedCGPath(const int id) const
{
    return generatedDir() + to_string(id) + (page(id).colorMode == PT_GRAY ? ".pgm" : ".ppm");
}

// bool Book::setPageWorkingIfReady(const int id)
// {
//     const Page& p = page(id);
//     if (p->status == PST_READY)
//     {
//         p->status = PST_WORKING;
//         emit app().pageStatusChanged(id);
//         return true;
//     }
//     return false;
// }
//
// void Book::invalidatePage(const int id)
// {
//     page(id)->status = PST_ERROR;
//     std::remove((generatedDir() + to_string(id) + "*").c_str());
//     emit app().pageStatusChanged(id);
// }
//
// void Book::validatePage(const int id)
// {
//     const Page& p = page(id);
//     p->lastStep = p->nextStep();
//     switch (p->lastStep.value())
//     {
//     case PS_CROPPING:
//         p->status = p->colorMode == PT_BLACK
//                         ? PST_WAITING
//                         : std::filesystem::exists(mergingChoicesDir() + to_string(id))
//                         ? PST_READY
//                         : PST_WAITING;
//         break;
//     case PS_MERGING:
//         p->status = PST_WAITING;
//         break;
//     case PS_CLEANING:
//         p->status = PST_READY;
//         break;
//     case PS_FINAL:
//         p->status = PST_COMPLETED;
//         break;
//     }
//     emit app().pageStatusChanged(id);
// }
//
// void Book::acceptChoice(const int id)
// {
//     const Page& p = page(id);
//     assert(p->status == PST_WAITING);
//     p->status = PST_READY;
//     emit app().choiceAccepted(id);
//     emit app().pageStatusChanged(id);
// }

string Book::getPageSourceThumbnail(const int id) const
{
    const Page& p = page(id);
    string path = sourcesThumbnailsdDir() + p.source + ".png";
    if (std::filesystem::exists(path)) // !
    {
        return path;
    }
    const auto pix = QImage(pageSourceBWPath(id).c_str());
    const QImage scaled = pix.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaled.save(path.c_str()); // !
    return path;
}

void Book::applyChoiceToPage(const int id, const SelectionInfo& selection)
{
    auto* step = _pages.at(id).nextStep();
    bool res = step->applyChoice(selection); // !
    emit choiceAccepted(id, res);
    if (!res)
    {
        step->status = SST_ERROR;
        emit pageStatusChanged(id);
    }
}
