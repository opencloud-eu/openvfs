#ifndef STRTOOLS_H
#define STRTOOLS_H

// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Klaas Freitag <k.freitag@opencloud.eu>

#pragma once
#include <string>
#include <vector>

namespace StrTools {
std::vector<std::string> split(const std::string &str, char delimiter);
}


#endif // STRTOOLS_H
