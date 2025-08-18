#ifndef LOGGER_HPP
#define LOGGER_HPP

#ifndef NDEBUG
// NOLINTBEGIN
#include <fmt/base.h>
#include <fmt/color.h>
#define NAME_OF(x) #x
#define LOG(msg)                                                               \
    fmt::println(                                                              \
        stderr, "{} : {} : {} : {} - {}",                                      \
        fmt::format(fmt::fg(fmt::color::red), "[LOG]"), __FILE__, __LINE__,    \
        NAME_OF(msg), msg                                                      \
    )
// NOLINTEND
#else
#define LOG(msg)
#endif

#endif
