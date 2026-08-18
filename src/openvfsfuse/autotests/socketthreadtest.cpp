// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Klaas Freitag <k.freitag@opencloud.eu>

/*
 * Drives SocketThread and SharedMap over a real AF_UNIX socket, standing in for
 * the desktop client. Covers the stream framing and the hydration wait, both of
 * which are timing dependent and regress silently.
 */

#include "sharedmap.h"
#include "socketthread.h"

#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

using namespace std::chrono_literals;

namespace {

int failures = 0;

void check(bool ok, const std::string &what)
{
    std::cerr << (ok ? "ok   : " : "FAIL : ") << what << std::endl;
    if (!ok) {
        ++failures;
    }
}

/// A socket path has to fit into sockaddr_un::sun_path, so keep it short and
/// out of the (potentially deeply nested) build directory.
std::string makeSocketPath()
{
    char tmpl[] = "/tmp/openvfs-test-XXXXXX";
    const char *dir = mkdtemp(tmpl);
    if (!dir) {
        std::cerr << "Failed to create a temporary directory" << std::endl;
        std::exit(2);
    }
    return std::string(dir) + "/s";
}

int listenOn(const std::string &path)
{
    const int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
    if (bind(fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0 || listen(fd, 1) != 0) {
        std::cerr << "Failed to listen on " << path << ": " << std::strerror(errno) << std::endl;
        std::exit(2);
    }
    return fd;
}

void writeAll(int fd, const std::string &data)
{
    size_t offset = 0;
    while (offset < data.size()) {
        const ssize_t n = write(fd, data.data() + offset, data.size() - offset);
        if (n <= 0) {
            return;
        }
        offset += static_cast<size_t>(n);
    }
}

std::string hydrateResult(int id, const std::string &argumentsBody)
{
    return "V2/HYDRATE_FILE_RESULT:{\"id\":\"" + std::to_string(id) + "\",\"arguments\":{" + argumentsBody + "}}\n";
}

}

int main()
{
    const std::string socketPath = makeSocketPath();
    const int server = listenOn(socketPath);

    SharedMap jobs;
    SocketThread socketThread("TestSocketThread", jobs);
    socketThread.CreateThread(socketPath);

    const int client = accept(server, nullptr, nullptr);
    if (client < 0) {
        std::cerr << "SocketThread did not connect" << std::endl;
        return 2;
    }

    // Register a job the way openVFSfuse_open() does, before posting it.
    const auto post = [&](int id) {
        auto data = std::make_shared<MsgData>();
        data->msg = "V2/HYDRATE_FILE";
        data->file = "/tmp/some/file";
        data->id = id;
        jobs.insert(id, HydJob{.state = HydJobState::Running});
        socketThread.PostMsg(data);
    };

    // A message split across two writes must not be acted on until it is complete.
    writeAll(client, "VERSION:1.2.3:2");
    std::this_thread::sleep_for(750ms);
    check(jobs.desktopClientPid() == 0, "an incomplete message is not dispatched");
    writeAll(client, ":4242\n");
    std::this_thread::sleep_for(750ms);
    check(jobs.desktopClientPid() == 4242, "a message split across two writes is reassembled");

    // A reply well past any single read buffer must arrive as one message. An
    // error string carrying a path and a description clears 1 KB easily.
    post(101);
    writeAll(client, hydrateResult(101, "\"error\":\"" + std::string(8000, 'E') + "\""));
    check(jobs.waitForJob(101, 10s) == HydJobResult::Failed, "an 8 KB reply is parsed as a single message");

    // Several replies batched into one write must all be dispatched.
    post(102);
    post(103);
    writeAll(client, hydrateResult(102, "\"status\":\"OK\"") + hydrateResult(103, "\"status\":\"FAILED\""));
    check(jobs.waitForJob(102, 10s) == HydJobResult::Succeeded, "first of two batched replies is dispatched");
    check(jobs.waitForJob(103, 10s) == HydJobResult::Failed, "second of two batched replies is dispatched");

    // ... and the stream is still in sync after all of the above.
    post(104);
    writeAll(client, hydrateResult(104, "\"status\":\"OK\""));
    check(jobs.waitForJob(104, 10s) == HydJobResult::Succeeded, "the stream stays in sync");

    // A silent client must time out, bounded by wall-clock time.
    post(105);
    const auto beforeTimeout = std::chrono::steady_clock::now();
    const auto timedOut = jobs.waitForJob(105, 500ms);
    const auto waited = std::chrono::steady_clock::now() - beforeTimeout;
    check(timedOut == HydJobResult::TimedOut, "a silent client is reported as a timeout");
    check(waited >= 450ms && waited < 30s, "the timeout is bounded by wall-clock time");

    // A reply arriving later must be observed without waiting out a long backoff.
    post(106);
    std::thread late([&] {
        std::this_thread::sleep_for(300ms);
        writeAll(client, hydrateResult(106, "\"status\":\"OK\""));
    });
    const auto beforeLate = std::chrono::steady_clock::now();
    const auto lateResult = jobs.waitForJob(106, 60s);
    const auto latency = std::chrono::steady_clock::now() - beforeLate;
    late.join();
    check(lateResult == HydJobResult::Succeeded, "a late reply is picked up");
    check(latency < 10s, "a late reply is not delayed by a growing backoff");

    // A caller must never be left waiting for a message that was never queued.
    socketThread.ExitThread();
    auto dropped = std::make_shared<MsgData>();
    dropped->msg = "V2/HYDRATE_FILE";
    dropped->id = 107;
    check(!socketThread.PostMsg(dropped), "PostMsg reports messages dropped during shutdown");

    close(client);
    close(server);
    std::error_code ec;
    std::filesystem::remove_all(std::filesystem::path(socketPath).parent_path(), ec);

    std::cerr << (failures == 0 ? "All checks passed" : std::to_string(failures) + " check(s) failed") << std::endl;
    return failures == 0 ? 0 : 1;
}
