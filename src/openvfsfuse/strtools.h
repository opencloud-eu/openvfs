// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Klaas Freitag <k.freitag@opencloud.eu>

#ifndef STRTOOLS_H
#define STRTOOLS_H

#pragma once
#include <string>
#include <vector>

namespace StrTools {

/*
 * join() takes a vector of strings and returns a string of all
 * entries of the vector joined together, delimited by the joiner
 * character
 */
std::string join(const std::vector<std::string> v, char joiner);

/*
 * split() takes a string, splits it at all occurances of delimiter
 * and returns a vector of strings containing all parts without the
 * delimiter
 */
std::vector<std::string> split(const std::string &str, char delimiter);
}


#endif // STRTOOLS_H
