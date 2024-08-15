//
// Created by benichn on 13/08/24.
//

#include "PageSettingsModel.h"
#include "../app.h"

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

json PageSettingsModel::createJson() const
{
    if (_pageId == -1) return {};
    ordered_json res = {};
    for (const auto& step : app().book().page(_pageId).steps)
    {
        const auto rs = step->realSettings();
        if (!rs.empty()) res[step->name()] = rs;
    }
    return res;
}

bool PageSettingsModel::editJsonProperty(const JsonStructure::path_t& path, const json& value) const
{
    return false;
}

json PageSettingsModel::createJsonDescriptor(const json& j) const
{
    return defaultDescriptor(j, {
                                 {"nullable", "true"}
                             });
}
