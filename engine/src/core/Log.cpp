#include "Log.hpp"

#include <cstdio>
#include <cstdlib>

namespace engine::log {

namespace {

const char* LevelTag(Level level) noexcept
{
    switch (level) {
        case Level::Trace: return "TRACE";
        case Level::Info:  return "INFO ";
        case Level::Warn:  return "WARN ";
        case Level::Error: return "ERROR";
        case Level::Fatal: return "FATAL";
    }
    return "?????";
}

// ANSI escape codes
const char* LevelColor(Level level) noexcept
{
    switch (level) {
        case Level::Trace: return "\033[37m";   // white
        case Level::Info:  return "\033[32m";   // green
        case Level::Warn:  return "\033[33m";   // yellow
        case Level::Error: return "\033[31m";   // red
        case Level::Fatal: return "\033[35m";   // magenta
    }
    return "\033[0m";
}

// Strip leading path components so only the filename is shown.
const char* BaseName(const char* path) noexcept
{
    const char* last = path;
    for (const char* p = path; *p; ++p) {
        if (*p == '/' || *p == '\\') last = p + 1;
    }
    return last;
}

} // namespace

void WriteImpl(Level level, const char* file, int line,
               const char* func, std::string_view msg)
{
    std::fprintf(stderr, "%s[%s] %s:%d (%s): %.*s\033[0m\n",
        LevelColor(level),
        LevelTag(level),
        BaseName(file),
        line,
        func,
        static_cast<int>(msg.size()),
        msg.data());

    if (level == Level::Fatal) {
        std::fflush(stderr);
        std::abort();
    }
}

} // namespace engine::log
