//
// Created by benichn on 13/08/24.
//

#include "PageSettingsModel.h"
#include "../app.h"
#include "../utils.h"

PageSettingsModel::PageSettingsModel(QObject* parent) : StaticJsonModel(parent)
{
    connect(&app().book(), &Book::globalSettingsChanged, this, &StaticJsonModel::resetJson);
    connect(&app().book(), &Book::bookReset, [=]
    {
        setSource(nullpropsource);
    });
}

void PageSettingsModel::setSource(const PropsSource& source)
{
    if (source != _source)
    {
        _source = source;
        resetJson();
    }
}

void PageSettingsModel::createJsonPlaceholder() const
{
}

void PageSettingsModel::createJson() const
{
    switch (_source.index())
    {
    case PTY_NONE:
        StaticJsonModel::createJson();
        StaticJsonModel::createJsonPlaceholder();
        break;
    case PTY_PAGE:
        {
            const int pageId = get<PTY_PAGE>(_source);
            auto pl = app().book().globalSettings();
            setJsonPlaceholder(pl);
            for (const auto& kv : pl.items())
            {
                const auto& step = app().book().page(pageId).step(kv.key());
                nullifyKeys(pl[kv.key()], step.settings);
                pl[kv.key()].update(step.settings);
            }
            setJson(pl);
        }
        break;
    }
}

void PageSettingsModel::afterEditJsonProperty(const JsonStructure::path_t& path, const json& value) const
{
    switch (_source.index())
    {
    case PTY_PAGE:
        {
            const int pageId = get<PTY_PAGE>(_source);
            app().book().setPageSettings(pageId, jSON());
        }
        break;
    }
}

void PageSettingsModel::createJsonDescriptor() const
{
    setJsonDescriptor(defaultDescriptor(placeholder(), {
                                            {"nullable", true}
                                        }));
}
