//
// Created by benichn on 13/08/24.
//

#include "PageSettingsModel.h"
#include "../app.h"
#include "../utils.h"

PageSettingsModel::PageSettingsModel(int pageId, QObject* parent) : StaticJsonModel(parent), _pageId(pageId)
{
}

PageSettingsModel::PageSettingsModel(QObject* parent) : PageSettingsModel(-1, parent)
{
}

void PageSettingsModel::setPageId(const int pageId)
{
    if (pageId != _pageId)
    {
        _pageId = pageId;
        resetJson();
    }
}

void PageSettingsModel::createJsonPlaceholder() const
{
}

void PageSettingsModel::createJson() const
{
    if (_pageId == -1)
    {
        StaticJsonModel::createJson();
        StaticJsonModel::createJsonPlaceholder();
    }
    else
    {
        auto pl = app().book().globalSettings();
        setJsonPlaceholder(pl);
        for (const auto& kv : pl.items())
        {
            const auto& step = app().book().page(_pageId).step(kv.key());
            nullifyKeys(pl[kv.key()], step.settings);
        }
        setJson(pl);
    }
}

bool PageSettingsModel::beforeEditJsonProperty(const JsonStructure::path_t& path, const json& value) const
{
    return false;
}

void PageSettingsModel::createJsonDescriptor() const
{
    setJsonDescriptor(defaultDescriptor(placeholder(), {
                                            {"nullable", true}
                                        }));
}
