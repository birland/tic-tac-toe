#ifndef BOARD_HPP
#define BOARD_HPP

#include <array>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/util/ref.hpp>
#include <functional>
#include <string>
#include <utility>
#include "player.hpp"
#include "timer.hpp"

class board {
public:
    explicit board(std::pair<player, player>* players, int size = 30);
    board(board const&)            = default;
    board(board&&)                 = default;
    board& operator=(board const&) = default;
    board& operator=(board&&)      = default;
    ~board()                       = default;

private:
    static constexpr unsigned buttons_count_{9};

public:
    using array_2d   = std::array<std::array<char, 3>, 3>;
    using buttons_2d = std::array<ftxui::Components, 3>;
    using button_2d  = std::array<ftxui::Component, buttons_count_ / 3>;
    using board_2d   = std::array<std::array<std::string, 3>, 3>;

    void                                   update_board();
    void                                   update_moves();
    board::buttons_2d&                     get_buttons();
    [[nodiscard]] board::buttons_2d const& get_buttons() const;

    button_2d&                     get_button_rows();
    [[nodiscard]] button_2d const& get_button_rows() const;
    [[nodiscard]] timer const&     get_timer() const;
    [[nodiscard]] timer&           get_timer();
    double                         get_secs_to_move();


    // Reference to the player which turn
    player& get_player_turn();

    void move_first();
    void player_move(auto& cell);
    void enemy_move();

    [[nodiscard]] bool    is_full();
    player::state_variant check_victory();

private:
    void init();
    ftxui::ButtonOption
    button_style(auto& label, std::function<void()> on_click);

    timer                                            timer_;
    double                                           prev_seconds_{};
    unsigned                                         enemy_move_delay_{2};
    int                                              selector_{};
    int                                              button_size_{};
    buttons_2d                                       buttons_;
    std::array<ftxui::Component, buttons_count_ / 3> button_rows_;
    std::pair<player, player>*                       players_;
    std::pair<bool, bool>                            move_turn_;
    bool                                             is_first_turn_{true};

    // std::array<std::string, 9>      board_{" ", " ", " ", " ", " ",
    //                                   " ", " ", " ", " "};
    // board_2d board_{{{' ', ' ', ' '}, {' ', ' ', ' '}, {' ', ' ', ' '}}};
    board_2d board_{{{" ", " ", " "}, {" ", " ", " "}, {" ", " ", " "}}};
};

#endif
