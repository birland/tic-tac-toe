#ifndef LABEL_BUTTONS_HPP
#define LABEL_BUTTONS_HPP

#include <array>
#include <cstdint>
#include <utility>

enum class label_idx : std::uint8_t {
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
static constexpr std::array<char const*, 9> labels_{"PLAY", "ABOUT", "OPTIONS",
                                                    "EXIT", "YES",   "NO",
                                                    "BACK", "SAVE",  "OK"};
static_assert(labels_.size() == std::to_underlying(label_idx::SIZE));

#endif
