# Contributing

Keep changes small and aligned with the milestone. M0 is local video I/O only.
Detection, tracking, weights, GPU APIs, and UI work require separate design and
evaluation. The core must not require cloud services, API keys, or uploads.

Windows/MSVC is the primary locally validated development/runtime platform.
Follow the Windows-first [README.md](README.md#build-and-test) setup to build and
test. Linux CI provides an independent portability/build check; it is not an
equally validated product target. Before submitting:

1. Configure into a fresh ignored build directory, build, and run CTest.
2. Run `git diff --check` and review the entire diff.
3. Check `git status --short` for media, outputs, build artifacts, secrets, and
   machine-specific paths. Never force-add ignored private files.
4. Describe changed behavior, validation, and backend limitations.

Use C++20, four-space indentation for C++/CMake, clear names, RAII, and explicit
errors. Compiler warnings are enabled; keep builds warning-free. M0 has no
mandatory formatter or external test framework. Tests should verify observable
behavior and generate their own synthetic fixtures.

The test executable supports `dashcam_tests --generate <local-fixture.avi>` and
`dashcam_tests --probe <local-video>` for local validation. Fixture generation
requires a fresh destination and refuses existing files, symlinks, and directories.
Do not share sensitive media or commit private filenames/identifying observations. See
[SECURITY.md](SECURITY.md) for reporting guidance.

Code uses Apache-2.0. Review third-party code/data/model licenses before proposing
dependencies. No model weights are part of M0.
