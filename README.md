# SRTDock

SRTDock is an open-source OBS Studio dock plugin for building and sharing SRT connection URLs.

It provides a compact Qt-based interface for configuring SRT caller, listener, and rendezvous connections.

## Features

- Caller, Listener, and Rendezvous modes
- Host / IP, port, and latency configuration
- Optional passphrase and stream ID parameters
- OBS Media Source URL generation in Listener mode
- Separate caller endpoint rows for Local and VPN addresses
- Copy-to-clipboard controls for generated URLs
- VPN availability checks for Tailscale and ZeroTier
- OBS-managed dock registration for reliable docking across OBS restarts
- GPL v2 licensed source code

## Requirements

- OBS Studio 31.1.1 or a compatible OBS Studio 31.x build
- Windows x64 for the currently tested build configuration
- Visual Studio 2022 with Desktop development with C++
- CMake 3.28 or newer
- Qt 6 development files matching the OBS Studio build
- OBS Studio development libraries and headers

The project uses the OBS Studio plugin CMake build structure and its dependency bootstrap files. Dependencies are downloaded or prepared by the build system; they are intentionally not committed to this repository.

## Easy Installation

The easiest way to install SRTDock on Windows is to download the installer from the latest GitHub Release:

[Download SRTDock Setup v0.1.0](https://github.com/masrezapamungkas/SRTDock/releases/download/v0.1.0/SRTDock_Setup_v0.1.0.exe)

Installation steps:

1. Close OBS Studio.
2. Run `SRTDock_Setup_v0.1.0.exe`.
3. Accept the license agreement.
4. Follow the installation wizard until it finishes.
5. Open OBS Studio again.
6. If the dock is not visible, open it from `View > Docks > SRTDock`.

The installer is intended for OBS Studio 64-bit installed in the standard Windows location. Administrator permission is required because the plugin is installed into the OBS Studio plugin directory.

## Build on Windows

Open a Visual Studio Developer Command Prompt for x64, then run:

```bat
cmake --preset windows-x64
cmake --build build_x64 --config RelWithDebInfo
```

The resulting plugin is generated under:

```text
build_x64\RelWithDebInfo\SRTDock.dll
```

To build a Release configuration:

```bat
cmake --build build_x64 --config Release
```

Do not perform an in-source build. Keep generated build output in `build_x64` or another directory that is excluded by `.gitignore`.

## Installing for local testing

Close OBS Studio before replacing a loaded plugin DLL. Copy the built DLL to the OBS 64-bit plugin directory:

```text
C:\Program Files\obs-studio\obs-plugins\64bit\SRTDock.dll
```

Restart OBS Studio. If the dock is not visible, open it from:

```text
View > Docks > SRTDock
```

For distribution, use a separately produced installer or package. Installer files are intentionally not part of this source repository.

## Tests

The repository includes lightweight regression checks for the mode-dependent UI behavior:

```bat
python tests\test_regression.py
python tests\test_caller_media_source_visibility.py
python tests\test_dock_startup_and_visibility.py
```

These checks validate source-level behavior such as caller URL visibility, dynamic-row cleanup, and single-path OBS dock registration.

## Project layout

```text
src/                  C++ plugin and Qt dock implementation
cmake/                OBS plugin CMake helpers
.github/              GitHub Actions workflows and build actions
build-aux/            Formatting and build helper scripts
data/                 OBS plugin data files
tests/                Regression checks
CMakeLists.txt        Main CMake project
CMakePresets.json     Configure/build presets
buildspec.json        Plugin metadata and dependency versions
LICENSE               GNU General Public License version 2
```

## GitHub Actions

The repository contains the OBS plugin template-style workflows for formatting and platform builds. Review the workflow configuration and repository secrets before enabling signing, notarization, release packaging, or other distribution jobs.

## License

SRTDock is distributed under the GNU General Public License, version 2. See [LICENSE](LICENSE).

OBS Studio, Qt, SRT, Tailscale, and ZeroTier are separate projects with their own licenses and trademarks.

## Acknowledgements

This project uses the OBS Studio plugin build conventions and CMake support files. Please retain the applicable upstream notices when modifying or redistributing those files.
