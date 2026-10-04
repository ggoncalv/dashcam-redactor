# Samples and test media

No private footage is stored here. Keep real clips outside the repository and
pass their local paths to the CLI. Never copy them here for convenience.

Tests generate original 64x48, 12-frame color/block animations using AVI/MJPG
and MOV/mp4v. The synthetic content contains no people, plates, locations, or
third-party media. Tests attempt to remove generated fixtures and outputs;
interrupted tests or cleanup failures can leave ignored files. CTest uses the
build directory. No download or private directory is required.

The fixture generator refuses existing destinations, including symlinks and
directories. Choose a fresh path for each invocation. Generated MOV/mp4v tests
do not validate real dashcam MOV codecs; see the
[validation boundary](../README.md#validation-boundary).

Private real-world MP4 footage has been tested locally on Windows and remains
outside the repository. Automated tests never require it.

After building tests:

Windows (the README's NMake Release build):

```powershell
.\build\dashcam_tests.exe --generate .\build\synthetic.avi
.\build\dashcam-redactor.exe .\build\synthetic.avi --output .\build\synthetic.mp4
.\build\dashcam_tests.exe --probe .\build\synthetic.mp4
```

Linux / CI:

```sh
./build/dashcam_tests --generate ./build/synthetic.avi
./build/dashcam-redactor ./build/synthetic.avi --output ./build/synthetic.mp4
./build/dashcam_tests --probe ./build/synthetic.mp4
```

For Windows Visual Studio generator Release builds use `build/Release/` executables.
Do not commit generated video, including synthetic output. Any future
redistributable samples need explicit provenance and appropriate licensing.
