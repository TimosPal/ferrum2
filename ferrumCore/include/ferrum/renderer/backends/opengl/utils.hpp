#pragma once

#include <glad/gl.h>

#include <sstream>
#include <string>
#include <string_view>

#include "ferrum/common/exception.hpp"

#ifndef NDEBUG
#define FE_GL(call)                                                                                \
    do {                                                                                           \
        call;                                                                                      \
        GLenum error = glGetError();                                                               \
        if (error != GL_NO_ERROR) {                                                                \
            std::ostringstream oss;                                                                \
            oss << "OpenGL error 0x" << std::hex << error << " at " << __FILE__ << ":" << __LINE__ \
                << " in call: " #call;                                                             \
            FE_THROW("{}", oss.str());                                                             \
        }                                                                                          \
    } while (0)
#else
#define FE_GL(call) call
#endif

namespace ferrum::renderer::opengl::detail {

inline void RequireValid(bool is_valid, std::string_view operation, std::string_view object) {
    FE_THROW_IF(!is_valid, "Cannot {}: {} has invalid ID (0)", operation, object);
}

template <typename GetParameter, typename GetLog>
std::string GetInfoLog(GLuint id, GetParameter get_parameter, GetLog get_log) {
    GLint length = 0;
    FE_GL(get_parameter(id, GL_INFO_LOG_LENGTH, &length));
    std::string log;
    if (length > 0) {
        log.resize(static_cast<std::size_t>(length));
        GLsizei written = 0;
        FE_GL(get_log(id, length, &written, log.data()));
        log.resize(static_cast<std::size_t>(written));
    }
    return log.empty() ? "driver returned no info log" : log;
}

}  // namespace ferrum::renderer::opengl::detail
