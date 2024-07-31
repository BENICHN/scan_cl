//
// Created by benichn on 31/07/24.
//

#ifndef JSONIMPORTS_H
#define JSONIMPORTS_H

#include <nlohmann/json.hpp>

#include "qtimports.h"

using nlohmann::json;

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

#endif //JSONIMPORTS_H
