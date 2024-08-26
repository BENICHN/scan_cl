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
    const auto& optName = path.back();
    auto& scanner = app().scanner();
    const auto& opts = scanner.currentOptions();
    const auto it = str::find(opts, optName, [](const auto& desc) { return desc.title; });
    if (it == opts.end()) return false;
    const auto sta = scanner.setOptionValueAt(it - opts.begin(), value);
    // ! info ignored
    return sta;
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
            {"name", opt.title},
        };
        switch (opt.type)
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
        switch (opt.constraint_type)
        {
        case SANE_CONSTRAINT_RANGE:
            {
                const auto& c = opt.constraintRange();
                if (opt.type == SANE_TYPE_FIXED)
                {
                    details["min"] = SANE_UNFIX(c.min);
                    details["max"] = SANE_UNFIX(c.max);
                    details["step"] = SANE_UNFIX(c.quant);
                }
                else
                {
                    details["min"] = c.min;
                    details["max"] = c.max;
                    details["step"] = c.quant;
                }
            }
            break;
        case SANE_CONSTRAINT_WORD_LIST:
            {
                details["choices"] = opt.constraintWordList() | stv::transform([&](const auto& w)
                {
                    return opt.type == SANE_TYPE_FIXED ? json(SANE_UNFIX(w)) : json(w);
                }) | str::to<json::array_t>();
            }
            break;
        case SANE_CONSTRAINT_STRING_LIST:
            {
                details["choices"] = opt.constraintStringList();
            }
            break;
        }
        switch (opt.unit)
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
        j["settable"] = SANE_OPTION_IS_SETTABLE(opt.cap) == SANE_TRUE;
        j["active"] = SANE_OPTION_IS_ACTIVE(opt.cap) == SANE_TRUE;
        res[opt.title] = j;
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
