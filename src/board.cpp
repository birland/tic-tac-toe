#include "board.hpp"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <fmt/base.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/canvas.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/util/ref.hpp>
#include <functional>
#include <string>
#include <utility>
#include "player.hpp"
#include "random.hpp"
#include "timer.hpp"

// TODO: Test this.
// Bad way for beep sound :D
#ifdef _WIN32
#include <Windows.h>
static void beep() { Beep(440, 1000); }

#elif __linux__

#include <cstdio>
static void beep() { system("beep -f 5000 -l 50 -r 2"); }

#else
static void beep() { fmt::println("\a"); }
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

board::board(std::pair<player, player>* players, int button_size) :
    button_size_(button_size), players_(players) {
    for (unsigned cnt{}; cnt < buttons_.size(); ++cnt) {
        buttons_[cnt].resize(buttons_.size());
    }

    if (players_->first.get_symbol() == 'X') {
        move_turn_.first = true;
    } else {
        move_turn_.second = true;
    }
}

void board::update_board() {
    for (unsigned col{}; col < buttons_.size(); ++col) {
        for (unsigned row{}; row < buttons_[col].size(); ++row) {
            auto& cell   = board_[col][row];
            auto& button = buttons_[col][row];

            move_first();

            button = Button(button_style(cell, [&cell, this] {
                if (move_turn_.first && cell == " ") {
                    player_move(cell);
                } else {
                    // TODO: sound alert?
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

void board::update_moves() {
    if (move_turn_.second) { enemy_move(); }
}

board::buttons_2d&       board::get_buttons() { return buttons_; }
board::buttons_2d const& board::get_buttons() const { return buttons_; }

board::button_2d&       board::get_button_rows() { return button_rows_; }
board::button_2d const& board::get_button_rows() const { return button_rows_; }

[[nodiscard]] timer const& board::get_timer() const { return timer_; }
[[nodiscard]] timer&       board::get_timer() { return timer_; }
double                     board::get_secs_to_move() {
    return enemy_move_delay_ - timer_.elapsed_seconds();
}


// TODO: Implement slider for enemy move from 1 second to 10 seconds


void board::move_first() {
    if (is_first_turn_ && move_turn_.second) {
        enemy_move();
        is_first_turn_    = false;
        move_turn_.second = false;
    }

    is_first_turn_ = false;
}


void board::player_move(auto& cell) {
    auto& player = players_->first;

    cell = player.get_symbol_str_v();

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
                    cell              = enemy.get_symbol_str_v();
                    return;
                }
            }
        }
    }
}

player& board::get_player_turn() {
    return move_turn_.first ? players_->first : players_->second;
}

[[nodiscard]] bool board::is_full() {
    unsigned count{};
    for (auto const& col : board_) { count += std::ranges::count(col, " "); }
    return count == 0;
}

player::state_variant board::check_victory() {
    // TODO: Implement algorithm to check winner
    return player::state::won{};
}

ftxui::ButtonOption
board::button_style(auto& label, std::function<void()> on_click) {
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
        } else if (label == players_->first.get_symbol_str_v()) {
            local_color = players_->first.get_color();
        } else {
            local_color = players_->second.get_color();
        }

        auto element = text(label) | center | ftxui::color(local_color) |
            border | ftxui::color(local_color);

        if (es.focused) {
            element = text(label) | center | ftxui::color(local_color) |
                border | ftxui::color(ftxui::Color::Yellow3);
        }

        return element | ftxui::flex;
    };

    return option;
}
