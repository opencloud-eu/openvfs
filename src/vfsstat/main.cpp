// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Hannah von Reth <h.vonreth@opencloud.eu>

#include "openvfs/openvfs.h"
#include "openvfs/xattr.h"


#include <filesystem>
#include <format>
#include <iostream>

namespace {
OpenVFS::PlaceHolderAttributes getPlaceholderAttribs(const std::filesystem::path &path)
{
    const auto data = Xattr::CPP::getxattr(path, std::string(OpenVFS::Constants::XAttributeNames::Data));
    return OpenVFS::PlaceHolderAttributes::fromData(path, data ? std::vector<uint8_t>{data.value().cbegin(), data.value().cend()} : std::vector<uint8_t>{});
}

void stat(const std::filesystem::directory_entry &entry)
{
    const auto data = getPlaceholderAttribs(entry.path());
    std::cout << std::format("{:<60} {:^4} {:>10} {:>10} {:>10} {:>10} {:<33} {}", entry.path().filename().native(), data.validate() ? "✅" : "❌", data.size,
        data.realSize(), OpenVFS::Constants::name(data.state), OpenVFS::Constants::name(data.pinState), data.etag, data.fileId)
              << std::endl;
}
}

int main(int argc, char *argv[])
{
    if (argc == 2) {
        const auto entry = std::filesystem::directory_entry(argv[1]);
        if (!entry.exists()) {
            std::cout << entry.path() << " does not exist" << std::endl;
            return -1;
        }
        if (const auto registration = OpenVFS::Registration::fromAttributes(entry.path())) {
            std::cout << std::format("OpenVfs Stat: '{}' Owner: '{}' OpenVFS Version: {}", entry.path().native(), registration.owner(), registration.version())
                      << std::endl;
        } else {
            std::cout << std::format("OpenVfs Stat: '{}' ({})", entry.path().native(), registration.error()) << std::endl;
        }
        std::cout << std::format(
            "{:<60} {:<4} {:<10} {:<10} {:<10} {:<10} {:<33} {}", "Path", "Valid", "Size", "ActualSize", "State", "PinState", "Etag", "FileId")
                  << std::endl;
        if (entry.is_directory()) {
            for (const auto &child : std::filesystem::directory_iterator(entry)) {
                stat(child);
            }
            return 0;
        } else {
            stat(entry);
        }
    }
    return -1;
}
