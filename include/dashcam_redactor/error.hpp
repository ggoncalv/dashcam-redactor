#pragma once

#include <stdexcept>
#include <string>

namespace dashcam_redactor {

enum class ExitCode { success = 0, usage = 2, input = 3, output = 4, processing = 5 };

class Error : public std::runtime_error {
public:
    Error(ExitCode code, const std::string& message) : std::runtime_error(message), code_(code) {}
    [[nodiscard]] ExitCode code() const noexcept { return code_; }

private:
    ExitCode code_;
};

} // namespace dashcam_redactor
