# openVFS - Virtual File System for Cloud Storage

openVFS is a framework that provides files on demand for the free desktop. It is based on the [FUSE](https://en.wikipedia.org/wiki/Filesystem_in_Userspace) filesystem layer.

Files on demand is a feature for files residing on a cloud storage system such as OpenCloud or Nextcloud. Rather than syncing all remote files locally, the files on demand system defers downloading file content until it is explicitly requested — for example, when a user opens a file. Until then, files are visible in the filesystem but their content is not present locally. Because most files occupy zero bytes on disk until accessed, the system is significantly more resource-efficient than a full sync.

The openVFS layer is based on Linux FUSE and is designed to work in conjunction with the cloud sync client.

It exposes a POSIX interface to user-space applications, proxying filesystem calls such as `open()` and `readdir()` and returning metadata that accurately represents the remote files.

To track synchronization state, openVFS uses the so-called pin state, which is stored in extended file attributes (xattrs) on each file.

When an application issues an `open()` call for a file whose content has not yet been downloaded, the FUSE layer initiates a download via the desktop client and blocks the call until the transfer completes. Download is delegated to the desktop client because the client holds all credentials required to authenticate against the server.

---

> **Note:** This is **experimental** code. It builds on the work of Rémi Flament (remipouak at gmail.com), whose [LoggedFS](https://github.com/rflament/loggedfs) filesystem monitoring tool served as the initial foundation.

Contributions and discussion are welcome — feel free to open issues or pull requests.

## Architecture

![Architecture Overview of Files on Demand](/fod.png?raw=true "Files on Demand")

## Usage

In production, the openvfs FUSE layer is managed automatically by the desktop client, which handles mounting and unmounting without user intervention.

For debugging and investigation, the layer can be started manually, provided a desktop client is running and syncing the target directory.

A typical invocation:

```
openvfs -d -i <config-file> -o <owner-string> -s <socket-file> /path/to/synced/dir
```

To inspect the mount state, use `mount | grep -e '^openvfs'`. To unmount, use `fusermount -d <mounted-dir>`. Root access is not required.

## Installation from source

Ensure that FUSE3 is installed on the system, including the development headers.

openVFS uses CMake as its build system. Refer to the [CMake documentation](https://cmake.org/documentation/) for build instructions.

## Integration

Details on integrating openVFS into a sync client are described in the [integration document](INTEGRATION.md).

## Contribute

The files on demand system was initially proposed by [OpenCloud](https://opencloud.eu) in April 2025 and is under consideration by the open-source cloud storage project [Nextcloud](https://www.nextcloud.com).

Feedback is welcome. Start a discussion or open an issue on GitHub for bug reports and feature requests.

