# Architecture

## Current: M0

`src/main.cpp` handles process I/O and exit codes. `src/cli.cpp` parses arguments
without backend dependencies. `src/video_io.cpp` owns OpenCV/FFmpeg decoding,
encoding, staging, and verification. Public headers expose filesystem paths and
a processing report, without OpenCV types. There are no premature detection or
tracking abstractions.

```text
local video input
  -> validate paths
  -> FFmpeg decode through OpenCV
  -> unchanged decoded BGR frame
  -> FFmpeg encode through OpenCV into owned local staging directory
  -> finalize and decode output to verify dimensions, count, and rate
  -> publish verified local output without overwriting existing paths
```

Frames are processed sequentially with bounded memory. RAII owns capture,
writer, and staging resources. The writer closes before verification. Exceptions
unwind resources; scoped cleanup attempts to remove owned temporary files, and
cleanup errors are reported. Cleanup is not guaranteed if removal fails or the
process is interrupted.

Windows is the primary locally validated platform. It publishes the closed,
verified file using `MoveFileExW` with flags set to zero: no replacement and no
cross-volume copy fallback. Staging beside the destination keeps the rename on
one volume. Hard-link support is not required on Windows, so FAT32/exFAT are not
excluded by publication; those volumes have not been tested locally.
See the [Windows API contract](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexw).

The secondary Linux/portability path retains exclusive same-filesystem hard-link
publication, followed by removal of the staging link. Standard C++ rename can
replace an existing destination on POSIX; an existence check before rename would
leave an overwrite race. Copying directly to the final name could expose partial
output. Both current publication paths fail if a destination appears during
processing, leave that destination untouched, and attempt cleanup of owned staging
files on ordinary failures, reporting cleanup errors. Linux output still requires
hard-link support and excludes FAT32/exFAT; wider Linux filesystem support is
future work. Neither path promises
crash/power-loss durability or protection against hostile changes to parent paths.

Only constant-rate MP4/mp4v or AVI/MJPG video output is supported. Audio and
metadata are omitted. Missing rates, odd dimensions, changing/unsupported frame
formats, count mismatches, and failed output checks are refused. Input metadata
is only a sanity check: OpenCV cannot always distinguish decoding failure from
EOF. Unknown input counts produce a warning. Full output decoding compensates
for `VideoWriter::write` having no per-frame success return; see the
[OpenCV writer API](https://docs.opencv.org/4.12.0/dd/d9e/classcv_1_1VideoWriter.html).

Tests generate moving-color fixtures and compare decoded frame order and
approximate pixels across both output codecs. They exercise parser/process
contracts, invalid paths/media, truncation, failed verification, cleanup, and
overwrite protection, including destinations created during processing.
No private fixture is needed.

## Future, not implemented

```text
video input
  -> decode
  -> frame pipeline
  -> [future face/plate detection]
  -> [future temporal tracking]
  -> [future redaction]
  -> encode
  -> local output
  -> [future human review before publication]
```

The video I/O boundary can evolve to direct FFmpeg or another measured backend
without spreading codec concerns into future detection/tracking/redaction.
Future stages should exchange explicit geometry/timing and detection results.
No model, GPU API, cloud API, GUI, authentication, or database is selected by M0.
Audio policy, variable-rate timestamps, Unicode paths, and wider Linux filesystem
support remain future decisions.
