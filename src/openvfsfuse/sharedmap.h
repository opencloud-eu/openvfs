// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2025 Klaas Freitag <k.freitag@opencloud.eu>

/*
 * openvfsfuse - a Fuse layer to handle virtual filesystem items of cloud storage
 * Copyright (C) 2025  Klaas Freitag <k.freitag@opencloud.eu>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SHAREDMAP_H
#define SHAREDMAP_H

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>

/**
 * The values a hydration job can take. A job starts out Running and is moved to
 * one of the other states by the socket thread.
 */
namespace HydJobState {
constexpr int Success = 0;
constexpr int Running = 1;
constexpr int Failed = -1;
}

struct HydJob
{
public:
    int state;
};

/**
 * Outcome of waiting for a hydration job to leave the Running state.
 */
enum class HydJobResult {
    Succeeded, ///< the client reported the file as hydrated
    Failed, ///< the client reported an error, or the request could not be sent
    TimedOut, ///< the client did not answer within the timeout
    Lost, ///< the job vanished from the map, which should not happen
};

class SharedMap
{
public:
    SharedMap();
    void insert(int key, const HydJob &value);

    bool get(int key, HydJob &outValue);
    bool remove(int id);
    bool set(int key, const HydJob &value);

    /**
     * Block until the job identified by @p key leaves the Running state, or
     * until @p timeout has elapsed.
     *
     * The wait is bounded by wall-clock time rather than by a number of polls,
     * and a state change is observed as soon as the socket thread publishes it.
     */
    HydJobResult waitForJob(int key, std::chrono::milliseconds timeout);

    void printAll();
    void setDesktopClientPid(long pid);
    long desktopClientPid();

private:
    std::map<int, HydJob> _data;
    std::mutex _mutex;
    std::condition_variable _cv;
    long _pid;
};


#endif // SHAREDMAP_H
