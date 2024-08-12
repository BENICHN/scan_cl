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
using std::ostringstream;
using std::runtime_error;

namespace stf = std::filesystem;
namespace str = std::ranges;
namespace stv = std::ranges::views;

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
