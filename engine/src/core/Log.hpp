#pragma once

#include <format>
#include <string_view>

namespace engine::log {

enum class Level { Trace, Info, Warn, Error, Fatal };

// Non-template sink — definition in Log.cpp.
void WriteImpl(Level level, const char* file, int line,
               const char* func, std::string_view msg);

// Template front-end: formats the message then calls WriteImpl.
// std::format_string performs compile-time format-string checking.
template<typename... Args>
inline void Write(Level level, const char* file, int line, const char* func,
                  std::format_string<Args...> fmt, Args&&... args)
{
    WriteImpl(level, file, line, func,
              std::format(fmt, std::forward<Args>(args)...));
}

} // namespace engine::log

// ---------------------------------------------------------------------------
// Public macros — the only way engine code should log.
// ---------------------------------------------------------------------------
#define LOG_TRACE(...) ::engine::log::Write(::engine::log::Level::Trace, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_INFO(...)  ::engine::log::Write(::engine::log::Level::Info,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_WARN(...)  ::engine::log::Write(::engine::log::Level::Warn,  __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_ERROR(...) ::engine::log::Write(::engine::log::Level::Error, __FILE__, __LINE__, __func__, __VA_ARGS__)
// LOG_FATAL prints the message then calls std::abort().
#define LOG_FATAL(...) ::engine::log::Write(::engine::log::Level::Fatal, __FILE__, __LINE__, __func__, __VA_ARGS__)
