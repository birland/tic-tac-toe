#ifndef BOARD_HPP
#define BOARD_HPP

#include <array>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/util/ref.hpp>
#include <functional>
#include <utility>
#include "player.hpp"

class board {
public:
    explicit board(std::pair<player, player>* players, int size = 30);
    board(board const&)            = default;
    board(board&&)                 = default;
    board& operator=(board const&) = default;
    board& operator=(board&&)      = default;
    ~board()                       = default;
    using array_2d                 = std::array<std::array<char, 3>, 3>;
    using buttons_2d               = std::array<ftxui::Components, 3>;
    using board_2d                 = std::array<std::array<char, 3>, 3>;

    void                             update(std::function<void()> const& exit);
    board::buttons_2d&               get_buttons();
    std::array<ftxui::Component, 3>& get_button_rows();

private:
    static ftxui::ButtonOption button_style_default(int size);
    ftxui::ButtonOption        button_style_active(int size);

    int                                              selector_{};
    int                                              button_size_{};
    static constexpr unsigned                        buttons_count_{9};
    buttons_2d                                       buttons_;
    std::array<ftxui::Component, buttons_count_ / 3> button_rows_;
    std::pair<player, player>*                       players_;

    // std::array<std::string, 9>      board_{" ", " ", " ", " ", " ",
    //                                   " ", " ", " ", " "};
    // board_2d board_{{{' ', ' ', ' '}}, {' ', ' ', ' '}, {' ', ' ', ' '}};
    board_2d board_{{{' ', ' ', ' '}, {' ', ' ', ' '}, {' ', ' ', ' '}}};
};

#endif
