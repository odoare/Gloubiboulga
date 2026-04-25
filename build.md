## Building from Source

### Common prerequisites

*   CMake 3.22 or later
*   A C++ 17 compiler (see platform sections below)
*   The JUCE framework (version 8 or later), cloned as a sibling directory named `JUCE`:
    ```
    parent/
    ├── JUCE/
    └── Gloubiboulga/
    ```
*   The `FxmeJuceTools` module, placed at `../JUCE/usermodules/FxmeJuceTools`

---

### Linux

Install the required system packages (names may vary by distro):

```sh
# Debian / Ubuntu
sudo apt install libasound2-dev libx11-dev libxrandr-dev libxinerama-dev \
                 libxcursor-dev libfreetype-dev libfontconfig1-dev \
                 libgl1-mesa-dev libcurl4-openssl-dev
```

Configure and build:

```sh
cd Gloubiboulga
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Artefacts: `build/Gloubiboulga_artefacts/Release/VST3/` and `Standalone/`.

Install the VST3 to `~/.vst3`:

```sh
cmake --install build
```

---

### Windows

Install [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) (Community edition is free) with the **Desktop development with C++** workload, and [CMake](https://cmake.org/download/).

Open a **Developer Command Prompt for VS 2022** (or any terminal where the MSVC toolchain is on `PATH`), then:

```bat
cd Gloubiboulga
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

Alternatively, open the `build/` folder in Visual Studio after the configure step and build from the IDE.

Artefacts: `build\Gloubiboulga_artefacts\Release\VST3\` and `Standalone\`.

Install the VST3 to `%CommonProgramFiles%\VST3\`:

```bat
cmake --install build --config Release
```

---

### macOS

Install the Xcode Command Line Tools and [CMake](https://cmake.org/download/) (or via `brew install cmake`):

```sh
xcode-select --install
```

Configure and build:

```sh
cd Gloubiboulga
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

This produces both VST3 and AU (Audio Unit) bundles:

*   `build/Gloubiboulga_artefacts/Release/VST3/`
*   `build/Gloubiboulga_artefacts/Release/AU/`

Install both to the user plugin folders:

```sh
cmake --install build
```

For a universal binary (Apple Silicon + Intel) pass `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"` at configure time.

---

### Debug build (all platforms)

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

---

### Building a macOS binary without a Mac

Apple does not allow macOS to run on non-Apple hardware, so true cross-compilation is not a supported workflow. The practical options are:

*   **GitHub Actions (recommended):** The workflow at [`.github/workflows/build-macos.yml`](.github/workflows/build-macos.yml) runs automatically on every push. It uses an Apple Silicon runner and compiles a universal binary (arm64 + x86_64) in a single pass. The `.vst3` and `.component` bundles are uploaded as downloadable artifacts on the Actions tab.

*   **Mac rental:** Services such as [MacStadium](https://www.macstadium.com/) or [MacInCloud](https://www.macincloud.com/) rent hourly Mac instances accessible over SSH or VNC.

*   **osxcross (advanced):** The [osxcross](https://github.com/tpoechtrager/osxcross) project provides a macOS cross-compilation toolchain for Linux. It requires a copy of the macOS SDK (obtainable from Xcode). JUCE can build with it, but the setup is non-trivial and the AU format requires Apple's audio frameworks, which limits what can be tested outside real hardware.
