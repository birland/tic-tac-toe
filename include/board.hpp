#ifndef BOARD_HPP
#define BOARD_HPP

#include <array>
#include <cstdint>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/util/ref.hpp>
#include <functional>
#include <string>
#include <utility>
#include "player.hpp"

class board {
public:
    board(
        unsigned button_count, std::pair<player, player>* players, int size = 30
    );
    board(board const&)            = default;
    board(board&&)                 = default;
    board& operator=(board const&) = default;
    board& operator=(board&&)      = default;
    ~board()                       = default;
    using array_2d                 = std::array<std::array<char, 3>, 3>;

    void                             update(std::function<void()> exit);
    ftxui::Components&               get_buttons();
    std::array<ftxui::Component, 3>& get_button_rows();

private:
    ftxui::ButtonOption
    button_style_default(int size, std::function<void()> on_click);
    ftxui::ButtonOption button_style_active(int size);

private:
    int                                         selector_{};
    unsigned                                    button_count_{};
    int                                         button_size_{};
    ftxui::Components                           buttons_;
    [[maybe_unused]] std::pair<player, player>* players_;

    std::array<std::string, 9>      board_{" ", " ", " ", " ", " ",
                                      " ", " ", " ", " "};
    std::array<ftxui::Component, 3> button_rows_;
};

#endif
