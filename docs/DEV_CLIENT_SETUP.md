# Developer Setup for SquadOV's Client

## Prerequisites

Currently, we only support development on Windows 10.

* Visual Studio 2019
* CMake 3.15+
* Vcpkg (installed at `$VCPKG`)
* Node v14.16
* Yarn
* [Boost v1.74+](https://sourceforge.net/projects/boost/files/boost-binaries/1.74.0/)
* Perl

We shall assume that you have downloaded the SquadOVClient repository into `$SRC/SquadOVClient`.
We will assume all commands are run in Powershell, hence the availability of UNIX-esque commands.

## Environment Variables

* Set `BOOST_ROOT` to be the location of your Boost installation (e.g. `C:\local\boost_1_76_0`)

## Vcpkg Packages

* `openssl:x64-windows`
* `librdkafka:x64-windows`
* `c-ares:x64-windows`
* `openssl:x64-windows-static`
* `openssl:x64-windows-static-md`
* `cryptopp:x64-windows`
* `cryptopp:x64-windows-static`
* `cryptopp:x64-windows-static-md`

## Build SquadOV Client Service

* `cd $SRC`
* `mkdir build && cd build`
* `cmake -S ../ -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$VCPKG\scripts\buildsystems\vcpkg.cmake`
* `cmake --build build --config Debug --target squadov_client_service`

## SquadOV Common Web UI

* `cd $SRC\client_ui\common_web`
* `yarn install`


## SquadOV Desktop

* `cd $SRC\client_ui\common_web`
* `yarn build-desktop`
* `cd $SRC\client_ui\desktop`
* `yarn install`
* `yarn build`

There are a few environment variables that let you configure the SquadOV UI.
* `SQUADOV_SERVICE_EXE`: The path to the `squadov_client_service.exe`, e.g. `$SRC\build\bin\x64\Debug\squadov_client_service.exe` (mandatory).
    * ie. `$env:SQUADOV_SERVICE_EXE="$SquadOVClient\build\bin\x64\Debug\squadov_client_service.exe"`
* `SQUADOV_API_PORT`: Port to run the local API server on (recommended).
    * ie. `$env:SQUADOV_API_PORT="8080"`
* `SQUADOV_API_KEY`: Not currently used.
* `SQUADOV_ZEROMQ_SERVICE_PORT`: Port to run the ZeroMQ socket on for communication between the UI and the client service (recommended).
* `SQUADOV_MANUAL_SERVICE`: Set to `1` if you do not wish to have the SquadOV UI automatically run the specified client service EXE.
* `SQUADOV_APPDATA_SUFFIX`: The local SquadOV files are written to `%APPDATA%\SquadOV${SQUADOV_APPDATA_SUFFIX}`.

In Windows, you can set the environment variable by typing `$env:{NAME}={VALUE}`.
Finally, start the UI by running: `yarn start`.

## SquadOV Web

* `cd $SRC\client_ui\common_web`
* `yarn build-web`
* `cd $SRC\client_ui\web`

You will now want to create a `config\config.json` file from the `config\config.json.tmpl` template.
You will want to set:
* `apiUrl`: `http://localhost:8080`
* `usePort`: `true`
* `protocol`: `http`

* `yarn server .\config\config.json`