//
// Created by benichn on 29/07/24.
//

#include "book.h"

#include <utility>

#include "../app.h"
#include "../utils.h"
#include "../imports/opencvimports.h"

// const Step* Page::nextStep() const
// {
//     for (const Step* s : steps)
//     {
//         if (s->status() != SST_COMPLETE) return s;
//     }
//     return nullptr;
// }

Page::Page(const int id, PageColorMode color_mode, string source, const optional<string>& cg_source,
           const int sub_page): id(id),
                                colorMode(color_mode),
                                source(std::move(source)),
                                cgSource(cg_source),
                                subPage(sub_page)
{
    steps.emplace_back(make_unique<CroppingStep>(id));
    steps.emplace_back(make_unique<MergingStep>(id));
    steps.emplace_back(make_unique<CleaningStep>(id));
    steps.emplace_back(make_unique<FinalStep>(id));
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
        if (!s->enabled()) continue;
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
    emit pageListChanged();
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

void Book::removePage(int id)
{
    cleanPage(id);
    deletePageSourceIfNotUsed(id);
    _pages.erase(id);
    erase_if(_ids, [=](const auto elem) { return elem == id; });
    emit pageListChanged();
}

void Book::removePages(const vector<int>& ids)
{
    for (const auto id : ids)
    {
        cleanPage(id);
        deletePageSourceIfNotUsed(id);
        _pages.erase(id);
        erase_if(_ids, [=](const auto elem) { return elem == id; });
    }
    emit pageListChanged();
}

void Book::deletePageSourceIfNotUsed(int id)
{
    const auto& p = page(id);
    if (str::all_of(_pages, [&, id](const auto& kv)
    {
        return kv.second.id == id || kv.second.source != p.source;
    }))
    {
        stf::remove(pageSourceBWPath(id));
    }
    if (!p.cgSource.has_value()) return;
    if (str::all_of(_pages, [&, id](const auto& kv)
    {
        return kv.second.id == id || kv.second.cgSource != p.cgSource;
    }))
    {
        stf::remove(pageSourceCGPath(id));
    }
}

void Book::cleanPage(const int id)
{
    stf::remove(pageGeneratedBWPath(id));
    stf::remove(pageGeneratedCGPath(id));
    stf::remove(pageGeneratedBigsMaskPath(id));
    stf::remove(pageMergingMaskPath(id));
    stf::remove(pageChosenBigsPath(id));
}

void Book::resetPage(const int id)
{
    cleanPage(id);
    const auto& p = page(id);
    for (const auto& step : p.steps)
    {
        step->status = SST_NOTRUN;
    }
    emit pageStatusChanged(id);
}

void Book::setPageSettings(const int id, const json& json)
{
    for (const auto& step : _pages.at(id).steps)
    {
        const auto it = json.find(step->name());
        if (it != json.end())
        {
            step->settings = *it;
        }
    }
    emit pageSettingsChanged(id);
}

void Book::setPageSettings(const int id, const string& name, const json& json)
{
    _pages.at(id).step(name).settings = json;
    emit pageSettingsChanged(id);
}

string Book::savingPath() const
{
    return _root + "/book.json";
}

void Book::save()
{
    const json j = *this;
    ofstream file(savingPath());
    file << j.dump(2);
}

void Book::loadFromRoot(const string& root)
{
    close();
    ifstream file(root + "/book.json");
    const auto j = json::parse(file);
    _root = root;
    loadFromJson(j);
}

void Book::close()
{
    _root = "";
    _title = "";
    _globalSettings = json::object();
    _pages.clear();
    _ids.clear();
    _romanLimit = 0;
}

Book::Book(const string& root, const string& title, const json& global_settings, int roman_limit): _root(root),
    _title(title),
    _globalSettings(global_settings),
    _romanLimit(roman_limit)
{
}

Book Book::newBook(const string& root)
{
    return {
        root,
        "Sans titre",
        {
            {
                "Finalisation", {
                    {"alignmentH", "l"},
                    {"alignmentV", "t"},
                    {"bottomMarginCm", 1.4},
                    {"dpi", 600},
                    {"finalHeightCm", 23.3},
                    {"finalWidthCm", 15.0},
                    {"forcedBottomCm", -1.0},
                    {"forcedLeftCm", -1.0},
                    {"forcedRightCm", -1.0},
                    {"forcedTopCm", -1.0},
                    {"headerHeightCm", 2.75},
                    {"leftMarginCm", 1.7},
                    {"rightMarginCm", 1.7},
                    {"topMarginCm", 1.4},
                }
            },
            {
                "Recadrage", {
                    {
                        "blurSize", {
                            110,
                            12
                        }
                    },
                    {"colorGamma", 0.5},
                    {"cropOverflow", 50},
                    {"flip", true},
                    {"maxBigCCColorMean", 15000},
                    {"maxBlockDist", 60},
                    {"maxBlurredCCArea", 4000},
                    {"minBlockSize", 10},
                    {"minConnectedBlockSize", 250},
                    {"smallImageBlocksArea", 20},
                    {"whiteColorThreshold", 240},
                    {"whiteThreshold", 234}
                }
            }
        },
        0,
    };
}

Book::Book()
{
    connect(this, &Book::pageStatusChanged, this, &Book::save);
    connect(this, &Book::pageListChanged, this, &Book::save);
    connect(this, &Book::pageSettingsChanged, this, &Book::save);
    connect(this, &Book::romanLimitChanged, this, &Book::save);
    connect(this, &Book::titleChanged, this, &Book::save);
    connect(this, &Book::globalSettingsChanged, this, &Book::save);

    connect(this, &Book::bookReset, this, &Book::pageListChanged);
    connect(this, &Book::bookReset, this, [=] { emit romanLimitChanged(_romanLimit); });
    connect(this, &Book::bookReset, this, [=] { emit titleChanged(_title); });
    connect(this, &Book::bookReset, this, [=] { emit globalSettingsChanged(_globalSettings); });
}

json Book::globalSettings(const string& name) const
{
    if (_globalSettings.contains(name)) return _globalSettings.at(name);
    return json::object();
}

void Book::setGlobalSettings(const json& value)
{
    _globalSettings = value;
    emit globalSettingsChanged(_globalSettings);
}

void Book::setGlobalSettings(const string& name, const json& value)
{
    _globalSettings[name] = value;
    emit globalSettingsChanged(_globalSettings);
}

void Book::setTitle(const string& title)
{
    _title = title;
    emit titleChanged(_title);
}

void Book::setRomanLimit(int romanLimit)
{
    _romanLimit = romanLimit;
    emit romanLimitChanged(_romanLimit);
}

auto&& getOrCreate(auto&& dir)
{
    stf::create_directories(dir);
    return dir;
}

string Book::sourcesDir() const
{
    return getOrCreate(_root + "/sources/");
}

string Book::outputDir() const
{
    return getOrCreate(_root + "/output/");
}

string Book::generatedDir() const
{
    return getOrCreate(_root + "/.generated/");
}

string Book::sourcesThumbnailsdDir() const
{
    return getOrCreate(_root + "/.thumbnails/sources/");
}

string Book::mergingChoicesDir() const
{
    return getOrCreate(_root + "/.choices/merging/");
}

string Book::cleaningChoicesDir() const
{
    return getOrCreate(_root + "/.choices/cleaning/");
}

string Book::pageSourceBWPath(const int id) const
{
    return sourcesDir() + page(id).source;
}

bool Book::pageHasGenerated(const int id) const
{
    return stf::exists(pageGeneratedBWPath(id)); // !
}

string Book::pageMergingMaskPath(const int id) const
{
    return mergingChoicesDir() + to_string(id) + ".pbm";
}

bool Book::pageMergingMaskAvailable(const int id) const
{
    return stf::exists(pageMergingMaskPath(id));
}

bool Book::chooseMergingMask(const int id, const QImage& mask)
{
    return mask.save(pageMergingMaskPath(id).c_str());
}

bool Book::pageMixedAvailable(const int id) const
{
    return page(id).mergingStep().status == SST_COMPLETE;
}

QPixmap Book::pageGeneratedMixPixmap(const int id) const
{
    const auto& p = page(id);
    const bool color = p.colorMode == PT_COLOR;
    const auto flag = color ? cv::IMREAD_COLOR : cv::IMREAD_GRAYSCALE;

    const auto& fStep = p.finalStep();
    auto mask = imread(pageMergingMaskPath(id), cv::IMREAD_GRAYSCALE);
    if (fStep.status == SST_COMPLETE)
    {
        const auto margins = fStep.margins(mask.cols, mask.rows);
        mask = surroundWith(mask, margins.top(), margins.left(), margins.bottom(), margins.right(), 0);
    }
    auto bw = imread(pageGeneratedBWPath(id), flag);
    const auto cg = imread(pageGeneratedCGPath(id), flag);
    cg.copyTo(bw, mask);

    return QPixmap::fromImage({
        bw.data, bw.cols, bw.rows, bw.step, color ? QImage::Format::Format_BGR888 : QImage::Format_Grayscale8
    });
}

string Book::pageGeneratedBigsMaskPath(const int id) const
{
    return generatedDir() + to_string(id) + "_bigs.pbm";
}

string Book::pageChosenBigsPath(const int id) const
{
    return cleaningChoicesDir() + to_string(id) + ".txt";
}

bool Book::pageChosenBigsAvailable(const int id) const
{
    const auto path = pageChosenBigsPath(id);
    if (!stf::exists(path) || !pageHasGenerated(id)) return false;
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

bool Book::choosePageBigs(const int id, const vector<PickerElement>& elements)
{
    ofstream file(pageChosenBigsPath(id));
    file << calculateXXH3_64(pageGeneratedBigsMaskPath(id)) << '\n';
    for (int i = 0; i < elements.size(); ++i)
    {
        if (elements.at(i).selected) file << i << ' ';
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
//                         : stf::exists(mergingChoicesDir() + to_string(id))
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
    if (stf::exists(path)) // !
    {
        return path;
    }
    const auto pix = QImage(pageSourceBWPath(id).c_str());
    const auto scaled = pix.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaled.save(path.c_str()); // !
    return path;
}

string Book::scanDir() const
{
    return sourcesDir(); // getOrCreate(_root + "/.scan/");
}

string Book::getNewScanFilename() const
{
    const auto now_time_t = std::time(nullptr);
    const auto now_tm = *std::localtime(&now_time_t);
    ostringstream ss;
    ss << std::put_time(&now_tm, "%Y-%m-%d-%H-%M-%S");
    const auto d = scanDir();
    bool ok = true;
    do
    {
        for (const auto& f : stf::directory_iterator(d))
        {
            const auto fn = f.path().stem().string();
            if (fn == ss.str())
            {
                ok = false;
                ss << '_';
                break;
            }
        }
    }
    while (!ok);
    ss << ".png";
    return ss.str(); // d + ss.str();
}

string Book::getNewScanPath() const
{
    return scanDir() + getNewScanFilename();
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

void to_json(json& j, const Page& page)
{
    j["id"] = page.id;
    j["colorMode"] = page.colorMode;
    j["source"] = page.source;
    if (page.cgSource.has_value()) j["cgSource"] = page.cgSource.value();
    j["subPage"] = page.subPage;
    j["steps"] = str::to<vector<json>>(page.steps | stv::transform([](const auto& step) { return step->toJson(); }));
}

Page from_json(const json& j)
{
    Page res{
        j.at("id"),
        j.at("colorMode"),
        j.at("source"),
        atOpt<string>(j, "cgSource"),
        j.at("subPage")
    };
    const vector<json> js = j.at("steps");
    assert(js.size() == res.steps.size());
    for (int i = 0; i < js.size(); ++i)
    {
        res.steps[i]->restoreJson(js[i]);
    }
    return res;
}

void to_json(json& j, const Book& book)
{
    j["romanLimit"] = book._romanLimit;
    j["title"] = book._title;
    j["globalSettings"] = book._globalSettings;
    vector<json> pages(book._ids.size());
    for (int i = 0; i < book._ids.size(); ++i)
    {
        pages[i] = book._pages.at(book._ids[i]);
    }
    j["pages"] = pages;
}

void Book::loadFromJson(const json& j)
{
    _title = j.at("title");
    _romanLimit = j.at("romanLimit");
    _globalSettings = j.at("globalSettings");
    const vector<json>& pages = j.at("pages");
    for (const auto& page : pages)
    {
        Page p = from_json(page);
        auto id = p.id;
        _pages.emplace(id, std::move(p));
        _ids.emplace_back(id);
    }
    emit bookReset();
    // ! signals
}
