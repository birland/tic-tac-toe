#include "board.hpp"
#include <algorithm>
#include <array>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <functional>
#include <gsl/pointers>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include "player.hpp"
#include "random.hpp"
#include "timer.hpp"

// Bad way for beep sound :D
#ifdef _WIN32

#include <Windows.h>
#include <utilapiset.h>
static std::jthread beep() { return std::jthread(Beep, 440, 200); }

#elif __linux__

#include <cstdlib> // for system()
static std::jthread beep() {
    return std::jthread(system, "beep -f 5000 -l 50 -r 2");
} // NOLINT

#else

static void beep() { fmt::print(stderr, "\a"); }

#endif


using ftxui::border;
using ftxui::Button;
using ftxui::ButtonOption;
using ftxui::center;
using ftxui::Components;
using ftxui::EntryState;
using ftxui::EQUAL;
using ftxui::HEIGHT;
using ftxui::size;
using ftxui::text;
using ftxui::WIDTH;

board::board(board::players_ptr players, int button_size) :
    button_size_(button_size), players_(players) {
    for (unsigned cnt{}; cnt < buttons_.size(); ++cnt) {
        buttons_[cnt].resize(buttons_.size());
    }

    if (players_->first.get_symbol() == "X") {
        move_turn_.first = true;
    } else {
        move_turn_.second = true;
    }
}

void board::update_draw() {
    for (unsigned col{}; col < buttons_.size(); ++col) {
        for (unsigned row{}; row < buttons_[col].size(); ++row) {
            auto& cell   = board_[col][row];
            auto& button = buttons_[col][row];

            move_first();

            button = Button(button_style(cell, [&cell, this] {
                if (move_turn_.first && cell == " " && !is_end_) {
                    player_move(cell);
                } else {
                    beep();
                }
            }));
        }
    }

    button_rows_ = {
        {ftxui::Container::Horizontal(
             {buttons_[0][0], buttons_[0][1], buttons_[0][2]}, &selector_
         ),
         ftxui::Container::Horizontal(
             {buttons_[1][0], buttons_[1][1], buttons_[1][2]}, &selector_
         ),
         ftxui::Container::Horizontal(
             {buttons_[2][0], buttons_[2][1], buttons_[2][2]}, &selector_
         )}
    };
}

void board::update_check_moves() {
    if (!is_end_) {
        if (move_turn_.second) { enemy_move(); }

        if (check_col() || check_row() || check_diag() || check_antidiag() ||
            is_full()) {
            is_end_ = true;
            // Reset timer in the end of the game to give 2 seconds
            // to player when enemy is won
            timer_.reset();
        }
    }
}

board::button_2d&    board::get_button_rows() { return button_rows_; }
[[nodiscard]] timer& board::get_timer() { return timer_; }
double               board::get_secs_to_move() {
    return enemy_move_delay_ - timer_.elapsed_seconds();
}

[[nodiscard]] bool board::is_end() const { return is_end_; }

void board::move_first() {
    if (is_first_turn_ && move_turn_.second) {
        enemy_move();
        is_first_turn_    = false;
        move_turn_.second = false;
    }

    is_first_turn_ = false;
}


void board::player_move(std::string& cell) {
    auto& player = players_->first;

    cell = player.get_symbol();

    // enemy's turn
    move_turn_.first  = !move_turn_.first;
    move_turn_.second = !move_turn_.second;
    timer_.reset();
}

void board::enemy_move() {
    if (static_cast<unsigned>(timer_.elapsed_seconds()) >= enemy_move_delay_ ||
        is_first_turn_) {
        auto& enemy = players_->second;

        for (unsigned arr_idx{}; arr_idx < board_.size(); ++arr_idx) {
            for (unsigned str_idx{}; str_idx < board_[arr_idx].size();
                 ++str_idx) {
                auto& cell = board_[psdrng::get(0U, 2U)][psdrng::get(0U, 2U)];
                if (cell == " ") {
                    // player's turn
                    move_turn_.first  = !move_turn_.first;
                    move_turn_.second = !move_turn_.second;
                    cell              = enemy.get_symbol();
                    return;
                }
            }
        }
    }
}

player* board::get_player_turn() {
    return move_turn_.first ? &players_->first : &players_->second;
}

[[nodiscard]] bool board::is_full() {
    unsigned count{};
    for (auto const& col : board_) {
        count += static_cast<unsigned>(std::ranges::count(col, " "));
    }
    return count == 0;
}

ftxui::ButtonOption
board::button_style(std::string& label, std::function<void()> on_click) {
    ButtonOption option;
    option.label    = label;
    option.on_click = std::move(on_click);

    option.animated_colors.background.Set(
        ftxui::Color::Default, ftxui::Color::Default
    );
    option.animated_colors.foreground.Set(
        ftxui::Color::Default, ftxui::Color::Yellow3
    );

    option.transform = [this, &label](EntryState const& es) {
        auto         wsize = size(WIDTH, EQUAL, button_size_);
        auto         hsize = size(HEIGHT, EQUAL, button_size_);
        ftxui::Color local_color;
        if (label == " ") {
            local_color = ftxui::Color::Green;
        } else if (label == players_->first.get_symbol()) {
            local_color = players_->first.get_color();
        } else {
            local_color = players_->second.get_color();
        }

        auto element = text(label) | center | ftxui::color(local_color) |
            border | ftxui::color(local_color);

        if (es.focused) {
            element = text(label) | center | ftxui::color(local_color) |
                border | ftxui::color(ftxui::Color::Yellow3) | ftxui::bold;
        }

        return element | ftxui::flex;
    };

    return option;
}

player::state_variant board::check_victory() {
    player::state_variant var;

    check_win_state(var);

    return var;
}


void board::check_win_state(player::state_variant& var) const {
    if (players_->first.is_won()) {
        var = player::state::won{};
    } else if (players_->second.is_won()) {
        var = player::state::losed{};
    } else {
        var = player::state::draw{};
    }
}

bool board::check_winner(std::string_view cell) {
    if (cell == " ") { return false; }

    if (cell == players_->first.get_symbol()) {
        ++counters_.first;
    } else if (cell == players_->second.get_symbol()) {
        ++counters_.second;
    }

    if (counters_.first == 3) {
        players_->first.set_won();
        return true;
    }
    if (counters_.second == 3) {
        players_->second.set_won();
        return true;
    }

    return false;
}

bool board::check_col() {
    for (unsigned col{}; col < board_.size(); ++col) {
        for (unsigned row{}; row < board_[col].size(); ++row) {
            auto& cell = board_[row][col];
            if (check_winner(cell)) { return true; }
        }
        counters_ = {};
    }

    return false;
}

bool board::check_row() {
    for (unsigned col{}; col < board_.size(); ++col) {
        for (unsigned row{}; row < board_[col].size(); ++row) {
            auto& cell = board_[col][row];
            if (check_winner(cell)) { return true; }
        }
        counters_ = {};
    }

    return false;
}

bool board::check_diag() {
    std::array<std::string_view const, arr_2d_size> const diag{
        board_[0][0], board_[1][1], board_[2][2]
    };

    auto res = std::ranges::any_of(diag, [this](auto elem) {
        return check_winner(elem);
    });

    counters_ = {};

    return res;
}

bool board::check_antidiag() {
    std::array<std::string_view const, arr_2d_size> const antidiag{
        board_[0][2], board_[1][1], board_[2][0]
    };

    auto res = std::ranges::any_of(antidiag, [this](auto const& elem) {
        return check_winner(elem);
    });

    counters_ = {};

    return res;
}
