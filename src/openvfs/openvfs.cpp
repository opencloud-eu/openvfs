// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2025 Hannah von Reth <h.vonreth@opencloud.eu>

#include "openvfs/openvfs.h"

#include "openvfs/xattr.h"

#include "nlohmann/json.hpp"

#include <format>
#include <iostream>

OpenVFS::Registration::Registration(const std::filesystem::path &absolutePath, const std::string &owner)
    : _path(absolutePath)
    , _owner(owner)
{
}
OpenVFS::Registration::Registration(const std::filesystem::path &absolutePath)
    : _path(absolutePath)
{
}

std::vector<uint8_t> OpenVFS::Registration::toData() const
{
    return nlohmann::json::to_msgpack(
        {{Constants::name(Constants::RegistrationAttributes::Owner), _owner}, {Constants::name(Constants::RegistrationAttributes::Version), _version}});
}

OpenVFS::Registration OpenVFS::Registration::fromData(const std::filesystem::path &absolutePath, const std::vector<uint8_t> &d)
{
    if (d.empty()) {
        Registration out(absolutePath);
        out._error = "Invalid path";
        return out;
    }
    const auto j = nlohmann::json::from_msgpack(d);
    auto info = Registration(absolutePath, j.at(Constants::name(Constants::RegistrationAttributes::Owner)));
    info._version = j.at(Constants::name(Constants::RegistrationAttributes::Version));
    return info;
}

OpenVFS::Registration OpenVFS::Registration::fromAttributes(const std::filesystem::path &absolutePath)
{
    const auto data = Xattr::CPP::getxattr(absolutePath, OpenVFS::Constants::XAttributeNames::Registration);
    if (!data.has_value()) {
        Registration out(absolutePath);
        out._error = "No registration info found";
        return out;
    }
    return fromData(absolutePath, std::vector<uint8_t>{data.value().cbegin(), data.value().cend()});
}

OpenVFS::Registration OpenVFS::Registration::registerFilesystem(const std::filesystem::path &path, const std::string &owner)
{
    if (auto info = fromAttributes(path); info && info._owner != owner) {
        info._error = std::format("Failed to register OpenVFS. The path '{}' is owned by: {}", path.native(), info._owner);
        return info;
    }
    auto info = Registration(path, owner);
    const auto registration = info.toData();
    if (Xattr::setxattr(
            path.c_str(), Constants::XAttributeNames::Registration.data(), reinterpret_cast<const char *>(registration.data()), registration.size(), 0)
        < 0) {
        info._error = "Failed to register OpenVFS";
        return info;
    }
    return info;
}

bool OpenVFS::Registration::unregisterFilesystem(const Registration &info)
{
    if (Xattr::removexattr(info._path.c_str(), Constants::XAttributeNames::Registration.data()) < 0) {
        std::cerr << "Failed to unregister OpenVFS" << std::endl;
        return false;
    }
    // TODO: what to do with dehydtrated files
    return true;
}

std::vector<uint8_t> OpenVFS::PlaceHolderAttributes::toData() const
{
    assert(*this);
    return nlohmann::json::to_msgpack({{Constants::name(Constants::Attributes::Etag), etag}, {Constants::name(Constants::Attributes::FileId), fileId},
        {Constants::name(Constants::Attributes::Size), size}, {Constants::name(Constants::Attributes::State), state},
        {Constants::name(Constants::Attributes::PinState), pinState}});
}

OpenVFS::PlaceHolderAttributes OpenVFS::PlaceHolderAttributes::fromData(const std::filesystem::path &absolutePath, const std::vector<uint8_t> &d)
{
    if (d.empty()) {
        return {absolutePath};
    }
    const auto j = nlohmann::json::from_msgpack(d);
    return {absolutePath, j.at(Constants::name(Constants::Attributes::Etag)), j.at(Constants::name(Constants::Attributes::FileId)),
        j.at(Constants::name(Constants::Attributes::Size)), j.at(Constants::name(Constants::Attributes::State)),
        j.at(Constants::name(Constants::Attributes::PinState))};
}

std::optional<OpenVFS::PlaceHolderAttributes> OpenVFS::PlaceHolderAttributes::fromAttributes(const std::filesystem::path &absolutePath)
{
    const auto data = Xattr::CPP::getxattr(absolutePath, OpenVFS::Constants::XAttributeNames::Data);
    if (!data.has_value()) {
        std::cerr << "No placeholder attributes found for: " << absolutePath << std::endl;
        return std::nullopt;
    }
    return fromData(absolutePath, std::vector<uint8_t>{data.value().cbegin(), data.value().cend()});
}

std::size_t OpenVFS::PlaceHolderAttributes::realSize() const
{
    auto size = Xattr::CPP::getxattr(absolutePath, OpenVFS::Constants::XAttributeNames::RealSize);
    if (!size) {
        // the fuse layer is not available, so we can use the actual file size
        return std::filesystem::file_size(absolutePath);
    }
    return std::stol(size.value());
}
