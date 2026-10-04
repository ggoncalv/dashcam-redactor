#pragma once

#include <cstdint>
#include <filesystem>
#include <iosfwd>

namespace dashcam_redactor {

struct VideoReport {
    int width;
    int height;
    double fps;
    std::uint64_t frames;
    double elapsed_seconds;
};

// Decode, encode, and verify a local video without changing decoded pixels.
// Throws Error; an existing output is never overwritten.
[[nodiscard]] VideoReport transcode_video(const std::filesystem::path& input,
                                          const std::filesystem::path& output,
                                          std::ostream& progress);

} // namespace dashcam_redactor
