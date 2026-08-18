# OpenVFS Integration in the Desktop Client

## Starting the FUSE Layer

OpenVFS is designed to be integrated into a desktop sync client so that users do not need to manage the FUSE layer manually. The client is responsible for mounting and unmounting the FUSE layer as sync connections are created or removed.

A reference implementation is available in the [OpenVFS overlay plugin](https://github.com/opencloud-eu/desktop/blob/main/src/plugins/vfs/openvfs/vfs_openvfs.cpp) of the OpenCloud Desktop Client, specifically in the `startImpl()` method. That method is invoked from the [generic VFS code](https://github.com/opencloud-eu/desktop/blob/main/src/libsync/vfs/vfs.cpp) as a hook when the VFS plugin starts.

The openvfs binary accepts the following command-line parameters:

* `-d`: Enable debug logging. Output is forwarded to the desktop client log.
* `-i <config-file>`: Path to the configuration file.
* `-o <owner-string>`: An identifier written as an extended file attribute on the sync root directory. It associates the directory with the owning client instance to prevent conflicts between multiple clients.
* `-s <socket-file>`: Path to the socket file used for communication with the desktop client.

The FUSE layer must be unmounted when the desktop client shuts down or when a sync connection is removed.

## Configuration File

The file passed via `-i` is JSON. All keys are optional; a missing key falls back to the built-in default.

* `ignoreApps.byName`: List of absolute executable paths that must not trigger a hydration. A matching caller receives `EPERM` from `open()` instead of the file being downloaded.
* `ignoreApps.endsWith`: List of suffixes matched against the calling executable's path, for the same purpose. This is the distribution-independent way to name a binary, and `-thumbnailer` catches the freedesktop thumbnailer convention in one entry.
* `hydrationTimeoutSeconds`: How long `open()` blocks waiting for the client to hydrate a file before giving up and returning `ETIMEDOUT`. Defaults to `300`. Raise it if large files are legitimately transferred over slow links; the point of the bound is to keep an unresponsive client from wedging the calling application indefinitely.

The shipped `config.json` is a cross-desktop starting point, not an exhaustive list. Any indexer, thumbnailer, or antivirus that opens files as a matter of routine will otherwise download the whole sync root.

## Socket API Communication

OpenVFS communicates with the desktop client over a local, unauthenticated Unix socket — the so-called socket API. This bidirectional channel is already used by both OpenCloud and Nextcloud clients for file manager integration.

The following describes the messages sent by openVFS and the expected responses.

### 1. `VERSION`

Sent by openVFS on startup to retrieve the client version and process ID.

Expected response format:
```
VERSION:<client-version>:<api-version>:<pid>
```

openVFS extracts the PID from the third field of this response.

### 2. `V2/HYDRATE_FILE`

Sent by openVFS to instruct the desktop client to download a dehydrated file. Parameters are JSON-encoded and include:

* `file`: Absolute path of the file to hydrate.
* `fileId`: Remote file identifier used by the client to locate and download the file.
* `requester`: Name of the application requesting access to the file.
* `id`: Transfer identifier that the client must echo back in its response.

Expected response fields:

* `id`: The transfer identifier from the request.
* `status`: `OK` on success; any other value indicates failure.
* `arguments`: Optional object that may contain an `error` field describing the failure.

On a successful response, the file is considered hydrated and its content is available locally.

Messages are newline-delimited on a stream socket, so a reply may be split across several writes and several replies may be batched into one. openVFS reassembles them; the client need not size its replies to any particular limit.

If the client reports an error, `open()` fails with `EIO`. If it does not answer within `hydrationTimeoutSeconds`, `open()` fails with `ETIMEDOUT`.

