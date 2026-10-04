#include "dashcam_redactor/video_io.hpp"
#include "dashcam_redactor/error.hpp"

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cctype>
#include <iostream>
#include <random>
#include <string>
#include <system_error>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace dashcam_redactor {
namespace {
namespace fs = std::filesystem;

// Reserving a directory gives the encoder an exclusively owned staging path.
class StagedOutput {
public:
    StagedOutput(const fs::path& parent, const fs::path& extension) {
        std::random_device random;
        for (int attempt = 0; attempt < 32; ++attempt) {
            directory_ = parent / (".dashcam-redactor-" + std::to_string(random()) + "-" +
                                   std::to_string(random()));
            std::error_code error;
            if (fs::create_directory(directory_, error)) {
                file_ = directory_ / ("video" + extension.string());
                return;
            }
            if (error) {
                throw Error(ExitCode::output, "Cannot create temporary output: " + error.message());
            }
        }
        throw Error(ExitCode::output, "Cannot reserve a temporary output directory.");
    }

    StagedOutput(const StagedOutput&) = delete;
    StagedOutput& operator=(const StagedOutput&) = delete;

    ~StagedOutput() {
        // Only these two paths, created by this operation, may be removed.
        std::error_code error;
        fs::remove(file_, error);
        if (error) {
            std::cerr << "Warning: temporary video cleanup failed: " << error.message() << '\n';
        }
        error.clear();
        fs::remove(directory_, error);
        if (error) {
            std::cerr << "Warning: temporary directory cleanup failed: " << error.message() << '\n';
        }
    }

    [[nodiscard]] const fs::path& file() const { return file_; }

    void publish(const fs::path& output) const {
        std::error_code error;
#ifdef _WIN32
        // Rename the verified file on the same volume. With flags == 0 Windows
        // refuses existing destinations and never falls back to copying bytes.
        if (!MoveFileExW(file_.c_str(), output.c_str(), 0)) {
            error = std::error_code(static_cast<int>(GetLastError()), std::system_category());
        }
#else
        // Standard C++ rename may replace a destination on POSIX. Retain the
        // exclusive hard-link publication for the secondary portability build.
        fs::create_hard_link(file_, output, error);
#endif
        if (error) {
            throw Error(ExitCode::output,
                        "Cannot publish verified output without overwriting files: " + error.message());
        }
    }

private:
    fs::path directory_;
    fs::path file_;
};

int output_codec(const fs::path& output) {
    auto extension = output.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    if (extension == ".mp4") {
        return cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    }
    if (extension == ".avi") {
        return cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    }
    throw Error(ExitCode::output, "Unsupported output extension; use .mp4 or .avi.");
}

void verify_output(const fs::path& path, cv::Size size, double fps, std::uint64_t expected) {
    cv::VideoCapture capture(path.string(), cv::CAP_FFMPEG);
    if (!capture.isOpened()) {
        throw Error(ExitCode::processing, "Encoded output cannot be reopened.");
    }
    const double actual_fps = capture.get(cv::CAP_PROP_FPS);
    if (!std::isfinite(actual_fps) || std::abs(actual_fps - fps) > std::max(0.01, fps * 0.001)) {
        throw Error(ExitCode::processing, "Encoded output frame rate differs from the input.");
    }
    std::uint64_t frames = 0;
    cv::Mat frame;
    while (capture.read(frame)) {
        if (frame.empty() || frame.size() != size || ++frames > expected) {
            throw Error(ExitCode::processing, "Encoded output has unexpected frames or dimensions.");
        }
    }
    if (frames != expected) {
        throw Error(ExitCode::processing, "Encoded output frame count differs from decoded input.");
    }
}

} // namespace

VideoReport transcode_video(const fs::path& input, const fs::path& output, std::ostream& progress) {
    const auto start = std::chrono::steady_clock::now();
    std::error_code error;
    if (!fs::is_regular_file(input, error) || error) {
        throw Error(ExitCode::input, "Input must be an accessible regular local file.");
    }
    const auto output_status = fs::symlink_status(output, error);
    if (error && error != std::errc::no_such_file_or_directory) {
        throw Error(ExitCode::output, "Cannot inspect output path: " + error.message());
    }
    if (fs::exists(output_status)) {
        throw Error(ExitCode::output, "Output already exists; refusing to overwrite it.");
    }
    error.clear();
    const fs::path parent = output.has_parent_path() ? output.parent_path() : fs::path(".");
    if (!fs::is_directory(parent, error) || error || output.filename().empty()) {
        throw Error(ExitCode::output, "Output parent must be an accessible existing directory.");
    }
    const int codec = output_codec(output);
    try {
        cv::VideoCapture capture(input.string(), cv::CAP_FFMPEG);
        if (!capture.isOpened()) {
            throw Error(ExitCode::input,
                        "Cannot open input with OpenCV's FFmpeg backend. Check video validity "
                        "and that this OpenCV build includes FFmpeg.");
        }
        const double fps = capture.get(cv::CAP_PROP_FPS);
        if (!std::isfinite(fps) || fps <= 0) {
            throw Error(ExitCode::input, "Input has no usable frame rate; refusing to guess.");
        }
        const double reported_frames = capture.get(cv::CAP_PROP_FRAME_COUNT);
        cv::Mat frame;
        if (!capture.read(frame) || frame.empty()) {
            throw Error(ExitCode::input, "Input contains no decodable video frames.");
        }
        const cv::Size size = frame.size();
        if (size.width % 2 != 0 || size.height % 2 != 0) {
            throw Error(ExitCode::input, "Odd frame dimensions are unsupported; refusing to crop.");
        }
        if (frame.type() != CV_8UC3) {
            throw Error(ExitCode::input, "Expected decoded 8-bit, three-channel BGR frames.");
        }
        progress << "Input backend: " << capture.getBackendName() << "; " << size.width << 'x'
                 << size.height << "; " << fps << " fps; reported frames: " << reported_frames
                 << '\n';
        if (!std::isfinite(reported_frames) || reported_frames <= 0) {
            progress << "Warning: input frame count is unavailable; early decode termination "
                        "cannot be checked against metadata.\n";
        }
        StagedOutput staged(parent, output.extension());
        cv::VideoWriter writer;
        if (!writer.open(staged.file().string(), cv::CAP_FFMPEG, codec, fps, size, true)) {
            throw Error(ExitCode::output,
                        "Cannot open output encoder. Check codec support, permissions, and disk space.");
        }
        progress << "Output backend: " << writer.getBackendName() << "; encoding frames...\n";
        std::uint64_t frames = 0;
        do {
            if (frame.empty() || frame.size() != size || frame.type() != CV_8UC3) {
                throw Error(ExitCode::processing, "Decoded frame format changed during processing.");
            }
            // M0 passes each decoded frame through unchanged.
            writer.write(frame);
            ++frames;
            if (frames % 300 == 0) {
                progress << "Encoded " << frames << " frames.\n";
            }
        } while (capture.read(frame));
        if (std::isfinite(reported_frames) && reported_frames > 0 &&
            std::abs(reported_frames - static_cast<double>(frames)) > 0.5) {
            throw Error(ExitCode::processing,
                        "Decoded frame count differs from input metadata; input may be truncated "
                        "or the backend's frame count may be inaccurate.");
        }
        writer.release();
        capture.release();
        progress << "Verifying encoded output...\n";
        verify_output(staged.file(), size, fps, frames);
        staged.publish(output);
        const double seconds =
            std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        return {size.width, size.height, fps, frames, seconds};
    } catch (const cv::Exception& exception) {
        throw Error(ExitCode::processing, "OpenCV video processing failed: " + std::string(exception.what()));
    } catch (const fs::filesystem_error& exception) {
        throw Error(ExitCode::output, "Filesystem operation failed: " + std::string(exception.what()));
    }
}

} // namespace dashcam_redactor
