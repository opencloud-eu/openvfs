// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Klaas Freitag <k.freitag@opencloud.eu>
#include "strtools.h"

namespace StrTools {
// Custom implementation of string split, which is not available in std::
// remove it it once is added
std::string join(const std::vector<std::string> v, char joiner)
{
    std::string s;
    for (std::vector<std::string>::const_iterator ii = v.begin(); ii != v.end(); ++ii) {
        s += (*ii);
        if (ii + 1 != v.end()) {
            s += joiner;
        }
    }

    return s;
}

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
