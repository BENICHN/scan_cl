//
// Created by benichn on 31/07/24.
//

#ifndef JSONIMPORTS_H
#define JSONIMPORTS_H

#include <QVariant>
#include <nlohmann/json.hpp>

class QSize;

using nlohmann::json;
using nlohmann::ordered_json;
using nlohmann::json_pointer;
using json_value_t = nlohmann::detail::value_t;

void to_json(json& j, const QSize& v);
void from_json(const json& j, QSize& v);

namespace std
{
    template <typename T>
    void from_json(const json& j, optional<T>& opt)
    {
        if (j.is_null())
        {
            opt = nullopt;
        }
        else
        {
            opt = j.get<T>();
        }
    }

    template <typename T>
    void to_json(json& j, const optional<T>& opt)
    {
        if (opt)
        {
            j = *opt;
        }
        else
        {
            j = nullptr;
        }
    }
}

json qVariantToJson(const QVariant& v);

#endif //JSONIMPORTS_H
