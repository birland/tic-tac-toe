#ifndef LABEL_BUTTONS_HPP
#define LABEL_BUTTONS_HPP
#include <array>
#include <cstdint>
#include <fmt/format.h>

enum class Label : std::uint8_t {
    PLAY,
    ABOUT,
    OPTIONS,
    EXIT,
    YES,
    NO,
    BACK,
    SAVE,
    OK,
    SIZE
};
static constexpr std::array<char const*, 9> labels{"PLAY", "ABOUT", "OPTIONS",
                                                   "EXIT", "YES",   "NO",
                                                   "BACK", "SAVE",  "OK"};
static_assert(labels.size() == fmt::underlying(Label::SIZE));

#endif
