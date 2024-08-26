//
// Created by benichn on 31/07/24.
//

#include "jsonimports.h"
#include <QSize>

void to_json(json& j, const QSize& v)
{
    j = {
        v.width(),
        v.height()
    };
}

void from_json(const json& j, QSize& v)
{
    v = {
        j.at(0),
        j.at(1)
    };
}

json qVariantToJson(const QVariant& v)
{
    switch (v.typeId())
    {
    case QVariant::Bool:
        return v.toBool();
    case QVariant::Int:
        return v.toInt();
    case QVariant::UInt:
        return v.toUInt();
    case QVariant::LongLong:
        return v.toLongLong();
    case QVariant::ULongLong:
        return v.toULongLong();
    case QVariant::Double:
        return v.toDouble();
    case QVariant::Char:
        return static_cast<const char*>(v.data());
    case QVariant::String:
        return v.toString().toStdString();
    default:
        return nullptr;
    }
}

QVariant jsonToQVariant(const json& j)
{
    switch (j.type())
    {
    case nlohmann::detail::value_t::string:
        return j.get_ref<const json::string_t&>().c_str();
    case nlohmann::detail::value_t::boolean:
        return j.get<bool>();
    case nlohmann::detail::value_t::number_integer:
        return j.get<int>();
    case nlohmann::detail::value_t::number_unsigned:
        return j.get<uint>();
    case nlohmann::detail::value_t::number_float:
        return j.get<double>();
    default:
        return {};
    }
}

std::string dumpValue(const json& j)
{
    if (j.type() == json_value_t::string)
    {
        return j.get<std::string>();
    }
    return j.dump();
}
