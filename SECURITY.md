# Security and sensitive reports

**Do NOT attach private dashcam footage, faces, license plates, or other sensitive
video to public GitHub issues, pull requests, or security reports.** Do not post
private filenames, GPS/location information, identifying screenshots, credentials,
or raw logs with sensitive paths. Use generated synthetic reproductions and
sanitized technical diagnostics.

Use GitHub private vulnerability reporting if enabled on this repository. If it
is unavailable, ask for a private reporting channel with a non-sensitive
description; do not publish exploit details or private media. Do not attach
sensitive footage to private security reports either. Describe affected versions,
backend/build details, impact, and safe reproduction steps.

M0 is early development, not anonymization or a hostile media sandbox. Use
trusted regular local videos with maintained OpenCV/FFmpeg libraries. Native
third-party media parsers determine codec support and require security updates.
URLs, device capture, and playlist inputs are outside the supported contract.
No cloud processing, telemetry, credential handling, or uploads are implemented.

M0 retains visual identifiers and must not be published as redacted. Scoped
cleanup attempts to remove owned staging files and reports cleanup errors.
Cleanup failure, crashes, or forced termination may leave sensitive
temporary videos beside the destination. Keep source/staging/output directories
private and review output before publication.
