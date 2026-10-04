#include "dashcam_redactor/cli.hpp"
#include "dashcam_redactor/error.hpp"
#include "dashcam_redactor/video_io.hpp"

#include <exception>
#include <iostream>
#include <string_view>
#include <vector>

int main(int argc, char* argv[]) {
    using namespace dashcam_redactor;
    try {
        std::vector<std::string_view> arguments;
        for (int i = 1; i < argc; ++i) {
            arguments.emplace_back(argv[i]);
        }
        const auto options = parse_arguments(arguments);
        if (options.help) {
            std::cout << usage();
            return 0;
        }
        std::cout << "M0: no redaction is performed; audio and metadata are omitted.\n";
        const auto report = transcode_video(options.input, options.output, std::cout);
        std::cout << "Verified " << report.frames << " frames, " << report.width << 'x'
                  << report.height << " at " << report.fps << " fps in "
                  << report.elapsed_seconds << " seconds.\n";
        return 0;
    } catch (const Error& error) {
        std::cerr << "Error: " << error.what() << '\n';
        if (error.code() == ExitCode::usage) {
            std::cerr << usage();
        }
        return static_cast<int>(error.code());
    } catch (const std::exception& error) {
        std::cerr << "Unexpected failure: " << error.what() << '\n';
        return static_cast<int>(ExitCode::processing);
    }
}
