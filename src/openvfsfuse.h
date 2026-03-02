// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2023 Klaas Freitag <kfreitag@owncloud.com>
// SPDX-FileCopyrightText: 2025  Klaas Freitag <k.freitag@opencloud.eu>

/*
 * This work is based on the nice work of Rémi Flament - remipouak at gmail.com
 * called loggedFS: https://github.com/rflament/loggedfs
 */

#ifdef linux
/* For pread()/pwrite() */
#define _X_SOURCE 500
#endif

#include <filesystem>
#include <vector>

struct openVFSfuse_Args
{
    std::filesystem::path mountPoint; // where the users read files
    bool isDaemon = true; // true == spawn in background
    std::vector<std::string> fuseArgv;
    std::vector<std::string> appsNoHydrateFull; // these apps are not permitted to cause a dehydration
    std::vector<std::string> appsNoHydrateEndsWith;
};

int initializeOpenVFSFuse(openVFSfuse_Args &openVFSArgs);
void openvfsfuse_log(const std::string &path, const char *action, int returncode, const char *format, ...);
