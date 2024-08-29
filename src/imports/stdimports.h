//
// Created by benichn on 12/07/24.
//

#ifndef STDIMPORTS_H
#define STDIMPORTS_H

#include <bitset>
#include <iostream>
#include <fstream>
#include <memory>
#include <optional>
#include <queue>
#include <variant>
#include <vector>
#include <map>
#include <unordered_map>
#include <chrono>
#include <filesystem>
#include <generator>
#include <span>
#include <spanstream>

using std::cout;
using std::make_shared;
using std::make_unique;
using std::max;
using std::min;
using std::nullopt;
using std::string;
using std::shared_ptr;
using std::unique_ptr;
using std::static_pointer_cast;
using std::vector;
using std::bitset;
using std::variant;
using std::get;
using std::optional;
using std::queue;
using std::deque;
using std::map;
using std::unordered_map;
using std::to_string;
using std::ifstream;
using std::ofstream;
using std::basic_ifstream;
using std::basic_ofstream;
using std::istream;
using std::ostream;
using std::basic_istream;
using std::basic_ostream;
using std::istringstream;
using std::ostringstream;
using std::basic_istringstream;
using std::basic_ostringstream;
using std::runtime_error;
using std::in_place_index;
using std::generator;
using std::span;
using std::ispanstream;
using std::ospanstream;
using std::basic_ispanstream;
using std::basic_ospanstream;
using std::filesystem::path;

namespace stf = std::filesystem;
namespace str = std::ranges;
namespace stv = std::ranges::views;
namespace stc = std::chrono;

struct FilenameWithTimestamp
{
    string filename;
    stf::file_time_type lastWriteTime;

    friend bool operator==(const FilenameWithTimestamp& lhs, const FilenameWithTimestamp& rhs)
    {
        return lhs.filename == rhs.filename
            && lhs.lastWriteTime == rhs.lastWriteTime;
    }

    friend bool operator!=(const FilenameWithTimestamp& lhs, const FilenameWithTimestamp& rhs)
    {
        return !(lhs == rhs);
    }
};

#endif //STDIMPORTS_H
