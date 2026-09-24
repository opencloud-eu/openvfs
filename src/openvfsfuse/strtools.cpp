// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Klaas Freitag <k.freitag@opencloud.eu>
#include "strtools.h"

#include <ranges>
#include <string>

namespace StrTools {

std::string join(const std::vector<std::string> v, char joiner)
{
    return std::string{std::from_range, v | std::views::join_with(joiner)};
}

// Custom implementation of string split, which is not available in std::
// remove it it once is added
std::vector<std::string> split(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }

    tokens.push_back(str.substr(start));
    return tokens;
}

}
