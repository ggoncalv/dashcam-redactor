#include "dashcam_redactor/cli.hpp"
#include "dashcam_redactor/error.hpp"

namespace dashcam_redactor {

std::string_view usage() noexcept {
    return "Usage: dashcam-redactor <input> --output <output.mp4|output.avi>\n"
           "       dashcam-redactor --help\n"
           "M0: local video-only re-encoding. No redaction; audio is omitted.\n"
           "Output must not exist. MP4 uses mp4v; AVI uses MJPG.\n";
}

CliOptions parse_arguments(std::span<const std::string_view> arguments) {
    if (arguments.size() == 1 && (arguments[0] == "--help" || arguments[0] == "-h")) {
        return {{}, {}, true};
    }
    CliOptions options;
    bool have_output = false;
    bool positional_only = false;
    for (std::size_t i = 0; i < arguments.size(); ++i) {
        const auto argument = arguments[i];
        if (!positional_only && argument == "--output") {
            if (have_output || i + 1 == arguments.size() || arguments[i + 1].empty() ||
                arguments[i + 1].starts_with("--")) {
                throw Error(ExitCode::usage, "--output requires exactly one path.");
            }
            options.output = std::filesystem::path(arguments[++i]);
            have_output = true;
        } else if (!positional_only && argument == "--") {
            positional_only = true;
        } else if (!positional_only && argument.starts_with('-')) {
            throw Error(ExitCode::usage, "Unknown option. Use --help for usage.");
        } else if (!argument.empty() && options.input.empty()) {
            options.input = std::filesystem::path(argument);
        } else {
            throw Error(ExitCode::usage, "Expected exactly one input path.");
        }
    }
    if (options.input.empty() || !have_output) {
        throw Error(ExitCode::usage, "An input path and --output path are required.");
    }
    return options;
}

} // namespace dashcam_redactor
