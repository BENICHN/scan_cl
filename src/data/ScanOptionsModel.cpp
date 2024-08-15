//
// Created by benichn on 15/08/24.
//

#include "ScanOptionsModel.h"

#include "../app.h"

json ScanOptionsModel::createJson() const
{
    return waitFor(app().scanner().getOptionsValues().then([](const auto& res)
    {
        return res ? res.value() : json{};
    })); // ! blocking
}

bool ScanOptionsModel::editJsonProperty(const JsonStructure::path_t& path, const json& value) const
{
    return false;
}

json ScanOptionsModel::createJsonDescriptor(const json& j) const
{
    const auto& opts = app().scanner().currentOptions();
    json res;
    for (int i = 0; i < opts.size(); ++i)
    {
        if (i == 0) continue; // skip "number of options"
        const auto& opt = opts[i];
        json j{
            {"nullable", false},
        };
        switch (opt->type)
        {
        case SANE_TYPE_BOOL:
            j["type"] = "bool";
            break;
        case SANE_TYPE_INT:
            j["type"] = "int";
            break;
        case SANE_TYPE_FIXED:
            j["type"] = "float";
            break;
        case SANE_TYPE_STRING:
            j["type"] = "string";
            break;
        default:
            continue;
        }
        json details{};
        switch (opt->constraint_type)
        {
        case SANE_CONSTRAINT_RANGE:
            {
                auto* c = opt->constraint.range;
                details["min"] = opt->type == SANE_TYPE_FIXED ? SANE_UNFIX(c->min) : c->min;
                details["max"] = opt->type == SANE_TYPE_FIXED ? SANE_UNFIX(c->max) : c->max;
                details["step"] = opt->type == SANE_TYPE_FIXED ? SANE_UNFIX(c->quant) : c->quant;
            }
            break;
        case SANE_CONSTRAINT_WORD_LIST:
            {
                auto* c = opt->constraint.word_list;
                const int l = *c++;
                for (int k = 0; k < l; ++k)
                {
                    details["choices"].emplace_back(opt->type == SANE_TYPE_FIXED ? SANE_UNFIX(c[k]) : c[k]);
                }
            }
            break;
        case SANE_CONSTRAINT_STRING_LIST:
            {
                auto* c = opt->constraint.string_list;
                while (*c)
                {
                    details["choices"].emplace_back(*c);
                    ++c;
                }
            }
            break;
        }
        switch (opt->unit)
        {
        case SANE_UNIT_PIXEL:
            details["unit"] = "px";
            break;
        case SANE_UNIT_BIT:
            details["unit"] = "b";
            break;
        case SANE_UNIT_MM:
            details["unit"] = "mm";
            break;
        case SANE_UNIT_DPI:
            details["unit"] = "dpi";
            break;
        case SANE_UNIT_PERCENT:
            details["unit"] = "%";
            break;
        case SANE_UNIT_MICROSECOND:
            details["unit"] = "us";
            break;
        }
        if (!details.empty()) j["details"] = details;
        j["settable"] = SANE_OPTION_IS_SETTABLE(opt->cap) == SANE_TRUE;
        j["active"] = SANE_OPTION_IS_ACTIVE(opt->cap) == SANE_TRUE;
        res[opt->title] = j;
    }
    return res;
}

ScanOptionsModel::ScanOptionsModel(QObject* parent) : StaticJsonModel(parent)
{
    connect(&app().scanner(), &Scanner::currentOptionsChanged, [=]
    {
        resetJson();
    });
}
