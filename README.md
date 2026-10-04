# dashcam-redactor

A local-first C++ privacy tool being developed to detect and redact faces and
license plates in dashcam footage before publication. Raw footage is intended to
stay on your machine. The core requires no cloud services, API keys, or network
access for ordinary local video processing.

## Current status: M0, local video I/O

**M0 does not redact anything. Output retains visible faces, plates, and other
identifying details.** It decodes video frames, passes decoded pixels through
unchanged, re-encodes them, and verifies the output before publishing it locally.
Re-encoding is lossy; this is not a byte-for-byte copy. There is no detection,
tracking, inference, GPU integration, or GUI.

**Windows with MSVC is the primary development and runtime platform**, and is
currently validated locally. Linux CI is an independent portability/build check,
not an equally validated product target.

Automated redaction cannot guarantee anonymization. Output must be reviewed by
a person before publication, including after future redaction features arrive.

## Prerequisites

- Visual Studio / MSVC C++20 desktop build tools for the primary Windows setup.
  The secondary Linux build uses a C++20 compiler such as GCC 11+.
- CMake 3.20+ and a supported build tool.
- OpenCV 4 development headers/libraries with the **FFmpeg video I/O backend**.
  Only `core` and `videoio` are requested; no direct FFmpeg SDK is required.
  Codec availability depends on the installed OpenCV/FFmpeg build.
- A writable existing output directory. Windows publication uses a same-volume
  rename that refuses existing destinations; hard-link support is not required.
  The secondary Linux path requires hard-link support (for example ext4).

CMake and the application download no dependencies or models. CI uses Ubuntu
24.04 distribution packages and generates tiny synthetic videos.

## Validation boundary

MP4 input has been validated end to end against private real-world dashcam
footage on Windows with MSVC and OpenCV 4.12.0's FFmpeg backend. This represents
the developer's current real test corpus; no private footage is included here.
Synthetic tests cover AVI/MJPG inputs encoded to AVI/MJPG and MP4/mp4v, and a
generated MOV/mp4v round trip. Real-world MOV input has not been validated;
generated fixtures do not establish compatibility with other MOV codecs.
Other containers/codecs remain backend-dependent. The Ubuntu CI workflow is
configured but has not yet been executed; Linux behavior remains unvalidated.

## Build and test

### Windows (primary)

Install Visual Studio or Build Tools with the **Desktop development with C++**
workload, including the Windows SDK and CMake tools. Open an x64 Developer
PowerShell with MSVC and NMake on PATH. The locally validated CMake version is
4.2.0; the project minimum is 3.20.

Download and extract the official **OpenCV 4.12.0 Windows x64 package** from its
[versioned release](https://github.com/opencv/opencv/releases/tag/4.12.0), outside
the repository or under an ignored build directory. Replace `<opencv-install>`
below with the extracted directory containing `build`. The direct library
configuration avoids top-level MSVC-version detection differences. Keep both
OpenCV and its FFmpeg backend DLLs in the package's `bin` directory on PATH.

```powershell
$opencvRoot = '<opencv-install>'
$env:OpenCV_DIR = "$opencvRoot/build/x64/vc16/lib"
$env:PATH = "$opencvRoot/build/x64/vc16/bin;$env:PATH"
cmake -S . -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DOpenCV_DIR="$env:OpenCV_DIR" -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Match library architecture and build configuration; Debug requires corresponding
OpenCV Debug libraries/DLLs. A Visual Studio generator may also be used with a
CMake version supporting that Visual Studio: configure a separate directory with
`-A x64`, then use `--config Release` for build and `-C Release` for CTest.
`-DBUILD_TESTING=OFF` builds just the CLI/library.

M0 uses this versioned prebuilt package rather than a vcpkg manifest. A manifest
could pin a source-build dependency graph, but validating OpenCV/FFmpeg feature
selection and source builds would add setup and CI work. The official package
is the simpler validated Windows baseline for this milestone; existing vcpkg
installations are not required or modified. CMake still accepts an independently
supplied FFmpeg-enabled OpenCV via `OpenCV_DIR` or `CMAKE_PREFIX_PATH`. No local
dependency paths or binaries are committed.

### Linux / CI (secondary)

Ubuntu 24.04 CI installs distribution OpenCV packages and runs synthetic tests
as an independent portability/build check. It does not establish parity with
the locally validated Windows/private-MP4 setup. To reproduce that CI setup:

```sh
sudo apt-get update
sudo apt-get install --no-install-recommends -y build-essential cmake ninja-build libopencv-dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

These distribution packages are not pinned to the Windows OpenCV version.

## Usage

```powershell
.\build\dashcam-redactor.exe "<local-input>.mp4" --output "<new-output>.mp4"
.\build\dashcam-redactor.exe "<local-input>.mp4" --output "<new-output>.avi"
.\build\dashcam-redactor.exe --help
```

Windows Visual Studio generator Release builds use `build/Release/dashcam-redactor.exe`;
Linux builds use `./build/dashcam-redactor` with the same arguments.
Quote paths containing spaces. Options may precede the input. For input names
beginning with `-`, put options first and `--` before the input.

The output parent must already exist; the output path must not exist. Existing
files, symlinks, directories, and the input itself are refused. `.mp4` selects
MPEG-4 Part 2 (`mp4v`); `.avi` selects Motion JPEG (`MJPG`). Other output
extensions fail clearly, with no silent codec fallback. Input containers such
as `.mov` work only when the local backend can decode their contents. An
extension alone does not identify a codec.

The console reports backend, dimensions, frame rate, metadata frame count,
progress, verified frame count, and elapsed time. Diagnostics go to stderr.

| Exit code | Meaning |
| --- | --- |
| 0 | Success or help |
| 2 | Invalid CLI arguments |
| 3 | Missing/inaccessible/undecodable input or unsupported decoded format |
| 4 | Invalid output path, unavailable encoder, or publication/filesystem failure |
| 5 | Processing/verification failure or unexpected exception |

Backend exceptions are reported as processing failures.

## Limitations

- **Video only:** audio, subtitles, GPS, timestamps, and container metadata are
  omitted. M0 is unsuitable as an archival copy or evidentiary conversion.
- The backend-reported rate becomes a constant output frame rate. Variable frame
  timing is not preserved. Missing/invalid rates are refused rather than guessed.
  Verification permits small codec rounding (0.1% or 0.01 fps).
- Decoded dimensions are preserved. Backend rotation may honor source orientation
  metadata; orientation tags are not copied. Odd dimensions are refused because
  encoders may crop them. Decoded frames must be constant-size 8-bit BGR.
- OpenCV cannot reliably distinguish EOF from decoding failure. A positive
  reported input count that differs from decoded frames fails conservatively;
  inaccurate metadata can reject readable input. Unknown counts emit a warning,
  but early termination cannot be proven absent. See the
  [OpenCV capture API](https://docs.opencv.org/4.12.0/d8/dfe/classcv_1_1VideoCapture.html).
- Output is fully decoded again to catch silent writes, adding processing time.
  Verification checks count, dimensions, and rate, not original pixels/timing.
- Staging beside the destination needs roughly one output video's temporary
  space. Scoped cleanup attempts to remove owned staging files on normal
  completion and failure; cleanup errors are reported. Cleanup failure or forced
  termination can leave sensitive `.dashcam-redactor-*` directories; delete them
  locally when no job uses them. They are ignored by Git.
- Windows publishes the closed, verified staging file with a same-volume
  no-replace rename. FAT32/exFAT are not excluded by this mechanism, but have not
  been tested locally; filesystem permissions, capacity, and FAT32 file-size
  limits still apply. Linux retains exclusive hard-link publication, so Linux
  output requires hard-link support and excludes FAT32/exFAT. Both paths refuse
  destinations created during processing. No crash/power-loss durability is
  promised; wider Linux filesystem support remains future work.
- Non-ASCII paths on Windows depend on the narrow OpenCV filename API and process
  code page; portable Unicode paths are not established.
- Use trusted ordinary local video files. Playlist/network sources and hostile
  media sandboxing are outside the contract; see [SECURITY.md](SECURITY.md).

## Roadmap

1. **M0 (current):** decode/pass-through/encode, CLI, synthetic tests, CI.
2. **Future:** evaluate face/plate detection accuracy and licenses, prioritizing
   missed identifiers.
3. **Future:** temporal tracking and privacy-biased redaction.
4. **Future:** user-defined static privacy regions to obscure burned-in date/time,
   GPS coordinates, device IDs, and other fixed-position overlays.
5. **Future:** human review workflow and performance measurement; consider GPU
   inference only when measurements justify it.

See [architecture](docs/architecture.md), [privacy model](docs/privacy-model.md),
[contributing](CONTRIBUTING.md), and [sample guidance](samples/README.md).
Licensed under [Apache-2.0](LICENSE). Future model licenses require separate
review before integration.

## Third-party dependencies and licensing

The project's Apache-2.0 license covers its source, not dependency binaries.
[OpenCV](https://opencv.org/license/) 4.5+ uses Apache-2.0; earlier OpenCV 4.x
versions accepted by the build use BSD-3-Clause. Video I/O uses FFmpeg through
OpenCV. The official OpenCV 4.12.0 Windows bundle includes LGPL-2.1-or-later
FFmpeg notices; other FFmpeg builds may enable GPL components or nonfree options
with different redistribution conditions.

This source repository does not vendor OpenCV or FFmpeg. Before distributing
binaries, inventory the exact bundled dependencies, retain their licenses and
notices, and meet applicable source-availability and linking/relinking conditions.
Consult [FFmpeg's licensing guidance](https://ffmpeg.org/legal.html); the project's
Apache-2.0 license does not replace those obligations.
