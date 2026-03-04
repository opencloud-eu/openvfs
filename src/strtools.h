// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Klaas Freitag <k.freitag@opencloud.eu>

#pragma once
#include <cstdint>
#include <functional>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace StrTools {
std::vector<std::string> split(const std::string &str, char delimiter);

#define ENUM_CASE(X)                                                                                                                                           \
    case X:                                                                                                                                                    \
        return std::string(#X);

std::string printFlag(uint64_t flags);

template <typename T>
std::string printFlags(const std::string &name, T flags, const std::function<std::string(uint64_t)> &printer)
{
    std::stringstream out;
    out << name << "(";
    if (flags == 0) {
        out << printer(0);
    } else {
        const uint64_t value = static_ast<uint64_t>(flags);
        for (uint32_t p = 0; p < std::numeric_limits<T>::digits; ++p) {
            if (const uint64_t mask = uint64_t{1} << p; value & mask) {
                out << printer(mask) << "|";
            }
        }
    }
    out << ")";
    return out.str();
}
}
