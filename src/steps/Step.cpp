//
// Created by benichn on 01/08/24.
//

#include "Step.h"

#include "../app.h"

const Page& Step::page() const
{
    return app().book().page(_pageId);
}

Step::Step(const int pageId): _pageId(pageId)
{
}

json Step::realSettings() const
{
    auto res = app().book().globalSettings(name());
    if (settings.is_object()) res.update(settings);
    return res;
}

json Step::toJson()
{
    return {
        {"name", name()},
        {"settings", settings},
        {"status", status}
    };
}

void Step::restoreJson(const json& j)
{
    settings = j.at("settings");
    status = j.at("status");
}
