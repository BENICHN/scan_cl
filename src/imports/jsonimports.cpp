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