#pragma once

#include <filesystem>
#include <span>
#include <string_view>

namespace dashcam_redactor {

struct CliOptions {
    std::filesystem::path input;
    std::filesystem::path output;
    bool help = false;
};

[[nodiscard]] CliOptions parse_arguments(std::span<const std::string_view> arguments);
[[nodiscard]] std::string_view usage() noexcept;

} // namespace dashcam_redactor
