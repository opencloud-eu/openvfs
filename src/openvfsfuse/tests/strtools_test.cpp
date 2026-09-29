// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Klaas Freitag <k.freitag@opencloud.eu>

// Minimal, dependency-free unit tests for StrTools::join and StrTools::split.
// Each failed check reports a message and the process exits non-zero, which
// CTest interprets as a test failure.

#include "../strtools.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const std::string &message)
{
    if (!condition) {
        std::cerr << "FAILED: " << message << std::endl;
        ++failures;
    }
}

void testJoinBasic()
{
    const std::vector<std::string> input{"a", "b", "c"};
    check(StrTools::join(input, ',') == "a,b,c", "join basic three elements");
}

void testJoinSingleElement()
{
    const std::vector<std::string> input{"only"};
    check(StrTools::join(input, ',') == "only", "join single element");
}

void testJoinEmptyVector()
{
    const std::vector<std::string> input{};
    check(StrTools::join(input, ',') == "", "join empty vector yields empty string");
}

void testJoinWithEmptyStrings()
{
    const std::vector<std::string> input{"", "b", ""};
    check(StrTools::join(input, ',') == ",b,", "join preserves empty elements");
}

void testSplitBasic()
{
    const std::vector<std::string> expected{"a", "b", "c"};
    check(StrTools::split("a,b,c", ',') == expected, "split basic three elements");
}

void testSplitNoDelimiter()
{
    const std::vector<std::string> expected{"abc"};
    check(StrTools::split("abc", ',') == expected, "split with no delimiter present");
}

void testSplitEmptyString()
{
    const std::vector<std::string> expected{""};
    check(StrTools::split("", ',') == expected, "split of empty string yields one empty token");
}

void testSplitConsecutiveDelimiters()
{
    const std::vector<std::string> expected{"a", "", "b"};
    check(StrTools::split("a,,b", ',') == expected, "split with consecutive delimiters yields empty token");
}

void testSplitLeadingAndTrailingDelimiter()
{
    const std::vector<std::string> expected{"", "a", "b", ""};
    check(StrTools::split(",a,b,", ',') == expected, "split with leading and trailing delimiter");
}

void testJoinSplitRoundTrip()
{
    const std::vector<std::string> input{"one", "two", "three"};
    const auto joined = StrTools::join(input, '/');
    const auto splitBack = StrTools::split(joined, '/');
    check(splitBack == input, "join followed by split round-trips to original vector");
}

}

int main()
{
    testJoinBasic();
    testJoinSingleElement();
    testJoinEmptyVector();
    testJoinWithEmptyStrings();
    testSplitBasic();
    testSplitNoDelimiter();
    testSplitEmptyString();
    testSplitConsecutiveDelimiters();
    testSplitLeadingAndTrailingDelimiter();
    testJoinSplitRoundTrip();

    if (failures > 0) {
        std::cerr << failures << " test(s) failed." << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "All strtools tests passed." << std::endl;
    return EXIT_SUCCESS;
}
