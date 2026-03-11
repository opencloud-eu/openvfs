// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Hannah von Reth <h.vonreth@opencloud.eu>
#pragma once
#include "openvfs/openvfconstants.h"
#include "openvfs/openvfs_export.h"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace OpenVFS {
class OPENVFS_EXPORT Registration
{
public:
    Registration(const std::filesystem::path &absolutePath, const std::string &owner);

    [[nodiscard]] std::vector<uint8_t> toData() const;
    static Registration fromData(const std::filesystem::path &absolutePath, const std::vector<uint8_t> &d);
    static Registration fromAttributes(const std::filesystem::path &absolutePath);

    static Registration registerFilesystem(const std::filesystem::path &path, const std::string &owner);
    static bool unregisterFilesystem(const Registration &info);

    [[nodiscard]] const std::filesystem::path &path() const { return _path; }
    [[nodiscard]] const std::string &owner() const { return _owner; }
    [[nodiscard]] int version() const { return _version; }
    [[nodiscard]] const std::string &error() const { return _error; }

    [[nodiscard]] bool isOk() const { return _error.empty(); }
    operator bool() const { return isOk(); }


private:
    Registration(const std::filesystem::path &absolutePath);

    std::filesystem::path _path;
    std::string _owner;
    int _version = Constants::Version;
    std::string _error;
};

class OPENVFS_EXPORT PlaceHolderAttributes
{
public:
    /**
     * Creates a PlaceHolderAttributes object from a given absolute path.
     * It is assumed the file exists and is not a placeholder.
     * @param absolutePath
     */
    PlaceHolderAttributes(const std::filesystem::path &absolutePath)
        : absolutePath(absolutePath)
    {
    }

    std::filesystem::path absolutePath;
    std::string etag;
    std::string fileId;
    std::size_t size = 0;
    // assume an exisitng file by default
    Constants::States state = Constants::States::Hydrated;
    Constants::PinStates pinState = Constants::PinStates::Inherited;

    [[nodiscard]] bool isOk() const { return _ok; }

    operator bool() const { return isOk(); }

    [[nodiscard]] std::vector<uint8_t> toData() const;

    static PlaceHolderAttributes create(const std::filesystem::path &absolutePath, const std::string &etag, const std::string &fileId, std::size_t size)
    {
        using namespace OpenVFS::Constants;
        return {absolutePath, etag, fileId, size, States::DeHydrated, PinStates::Inherited};
    }

    static PlaceHolderAttributes fromData(const std::filesystem::path &absolutePath, const std::vector<uint8_t> &d);
    static std::optional<PlaceHolderAttributes> fromAttributes(const std::filesystem::path &absolutePath);

    [[nodiscard]] std::size_t realSize() const;

    [[nodiscard]] bool validate() const
    {
        if (!_ok) {
            return false;
        }
        switch (state) {
        case Constants::States::Hydrated:
            return size == 0;
        case Constants::States::DeHydrated:
            return realSize() == 0;
        case Constants::States::Hydrating:
            return true;
        }
        return true;
    }

private:
    PlaceHolderAttributes(const std::filesystem::path &absolutePath, const std::string &etag, const std::string &fileId, std::size_t size,
        Constants::States state, Constants::PinStates pinState)
        : absolutePath(absolutePath)
        , etag(etag)
        , fileId(fileId)
        , size(size)
        , state(state)
        , pinState(pinState)
        , _ok(true)
    {
    }

    bool _ok = false;
};
}
