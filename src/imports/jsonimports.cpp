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
        j[0],
        j[1]
    };
}