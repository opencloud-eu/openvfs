// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Hannah von Reth <h.vonreth@opencloud.eu>
// SPDX-FileCopyrightText: 2025 Klaas Freitag <k.freitag@opencloud.eu>

#include "openvfsfuse.h"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <optional>
#include <vector>

#include <nlohmann/json.hpp>

#include "strtools.h"

using json = nlohmann::json;

namespace {
constexpr int MaxFuseArgs = 32;

const std::string ConfigIgnoreAppsStr = "ignoreApps";
const std::string ConfigByNameStr = "byName";
const std::string ConfigEndsWith = "endsWith";
const std::string ConfigHydrationTimeoutSecondsStr = "hydrationTimeoutSeconds";

void usage(char *name)
{
    std::cerr << "Usage:" << std::endl //
              << name << " [-h] | [-f] [-p] [-d] -i config-file -o ownerId /directory-mountpoint" << std::endl //
              << "Type 'man openvfsfuse' for more details" << std::endl;
}

std::optional<openVFSfuse_Args> processArgs(int argc, char *argv[])
{
    openVFSfuse_Args out;
    // pass executable name through
    out.fuseArgv.emplace_back(argv[0]);
    opterr = 0;

    int res;

    // preset passed standard options which can not be set in fuse_config in
    // the init function called openVFSfuse_init()
    // auto_umount: Unmount the fuse layer automatically if the app crashes.
    std::vector<std::string> opts{"auto_unmount"};

    while ((res = getopt(argc, argv, "hpfdi:o:s:")) != -1) {
        switch (res) {
        case 'h':
            usage(argv[0]);
            return {};
        case 'f':
            out.isDaemon = false;
            // this option was added in fuse 2.x
            out.fuseArgv.emplace_back("-f");
            std::cout << "openVFSfuse not running as a daemon" << std::endl;
            break;
        case 'p':
            // make the mount public
            opts.push_back("allow_other");
            opts.push_back("default_permission");

            std::cout << "openVFSfuse running as a public filesystem" << std::endl;
            break;
        case 'd':
            // enable debug logging, implies -f
            out.fuseArgv.emplace_back("-d");
            out.debugEnabled = true;
            std::cout << "openVFSfuse running with debug log enabled" << std::endl;
            break;
        case 'i': {
            std::ifstream ifs(optarg);
            if (!ifs) {
                std::cerr << "Failed to open config file " << optarg << std::endl;
                return {};
            }
            json data;
            try {
                data = json::parse(ifs);
            } catch (const json::exception &e) {
                std::cerr << "Failed to parse config file " << optarg << ": " << e.what() << std::endl;
                return {};
            }

            // Every setting is optional so that a config file written for an
            // older version keeps working.
            const auto ignoreApps = data.value(ConfigIgnoreAppsStr, json::object());
            out.appsNoHydrateFull = ignoreApps.value(ConfigByNameStr, std::vector<std::string>{});
            out.appsNoHydrateEndsWith = ignoreApps.value(ConfigEndsWith, std::vector<std::string>{});

            const auto timeoutSeconds = data.value(ConfigHydrationTimeoutSecondsStr, 0);
            if (timeoutSeconds > 0) {
                out.hydrationTimeout = std::chrono::seconds(timeoutSeconds);
            } else if (timeoutSeconds < 0) {
                std::cerr << ConfigHydrationTimeoutSecondsStr << " must be positive, keeping the default" << std::endl;
            }
            break;
        }
        case 'o':
            out.owner = optarg;
            break;
        case 's':
            out.socketPath = optarg;
            break;
        default:
            assert(false);
            break;
        }
    }

    if (opts.size() > 0) {
        out.fuseArgv.emplace_back("-o");
        out.fuseArgv.emplace_back(StrTools::join(opts, ',').c_str());
    }

    if (optind + 1 <= argc) {
        out.mountPoint = std::filesystem::canonical(argv[optind++]);
        out.fuseArgv.emplace_back(out.mountPoint);
    } else {
        std::cerr << "Missing mountpoint" << std::endl;
        usage(argv[0]);
        return {};
    }

    // If there are still extra unparsed arguments, pass them onto FUSE..
    if (optind < argc) {
        assert(out.fuseArgv.size() < MaxFuseArgs);

        while (optind < argc) {
            assert(out.fuseArgv.size() < MaxFuseArgs);
            out.fuseArgv.emplace_back(argv[optind]);
            ++optind;
        }
    }

    if (!out.mountPoint.is_absolute()) {
        std::cerr << "You must use absolute paths (beginning with '/') for " << out.mountPoint << std::endl;
        return {};
    }

    return out;
}
}

int main(int argc, char *argv[])
{
    if (auto openvfsfuseArgs = processArgs(argc, argv)) {
        std::cout << "openVFSfuse starting at" << openvfsfuseArgs->mountPoint << "." << std::endl;
        return initializeOpenVFSFuse(*openvfsfuseArgs);
    }
    return -1;
}
