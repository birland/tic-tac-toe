#ifndef BOARD_HPP
#define BOARD_HPP

#include <array>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <functional>
#include <gsl/pointers>
#include <string>
#include <string_view>
#include <utility>
#include "player.hpp"
#include "timer.hpp"

class board {
public:
    using players_ptr = gsl::not_null<std::pair<player, player>*>;

    explicit board(players_ptr players, int size = 30);
    board(board const&)            = delete;
    board(board&&)                 = default;
    board& operator=(board const&) = delete;
    board& operator=(board&&)      = default;
    ~board()                       = default;

private:
    static constexpr unsigned buttons_count_{9};

public:
    static constexpr unsigned arr_size_{buttons_count_ / 3};
    using buttons_2d = std::array<ftxui::Components, arr_size_>;
    using button_2d  = std::array<ftxui::Component, arr_size_>;
    // strings instead of chars for ftxui labels ... >< can we fix this?
    using board_2d = std::array<std::array<std::string, arr_size_>, arr_size_>;
    ///////////////////////////////////////////////////////////////////////

    void                                   update_draw();
    void                                   update_moves();
    board::buttons_2d&                     get_buttons();
    [[nodiscard]] board::buttons_2d const& get_buttons() const;

    button_2d&                     get_button_rows();
    [[nodiscard]] button_2d const& get_button_rows() const;
    [[nodiscard]] timer const&     get_timer() const;
    [[nodiscard]] timer&           get_timer();
    double                         get_secs_to_move();
    [[nodiscard]] bool             is_end() const;


    // Reference to the player which turn
    player* get_player_turn();

    void move_first();
    void player_move(auto& cell);
    void enemy_move();

    [[nodiscard]] bool is_full();

private:
    void init();
    ftxui::ButtonOption
    button_style(auto& label, std::function<void()> on_click);

public:
    player::state_variant check_victory();

private:
    void check_winner(player::state_variant& var) const;
    bool check_cell(std::string_view cell);
    bool check_col();
    bool check_row();
    bool check_diag();
    bool check_antidiag();

    timer                                   timer_;
    double                                  prev_seconds_{};
    unsigned                                enemy_move_delay_{2};
    int                                     selector_{};
    int                                     button_size_{};
    buttons_2d                              buttons_;
    std::array<ftxui::Component, arr_size_> button_rows_;
    players_ptr                             players_;
    std::pair<unsigned, unsigned>           counters_;
    std::pair<bool, bool>                   move_turn_;
    bool                                    is_first_turn_{true};
    bool                                    is_end_{};

    /////////////////////
    //  |" "|" "|" "|  //
    //  |" "|" "|" "|  //
    //  |" "|" "|" "|  //
    /////////////////////
    board_2d board_{{{" ", " ", " "}, {" ", " ", " "}, {" ", " ", " "}}};
};

#endif
