//
// Created by benichn on 13/08/24.
//

#ifndef PROPSEDITORTYPES_H
#define PROPSEDITORTYPES_H

#include "../../imports/stdimports.h"

using PropsSource = variant<bool, int, int>;
enum PropsType
{
    PTY_NONE,
    PTY_PAGE,
    PTY_NUMBER
};

inline auto nullpropsource = PropsSource(in_place_index<0>);

#endif //PROPSEDITORTYPES_H
