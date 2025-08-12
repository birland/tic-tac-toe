#include "board.hpp"
#include "player.hpp"

#include <array>
#include <cstring>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/canvas.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/util/ref.hpp>
#include <functional>
#include <string>
#include <utility>

using ftxui::border;
using ftxui::Button;
using ftxui::ButtonOption;
using ftxui::Components;
using ftxui::EntryState;
using ftxui::EQUAL;
using ftxui::HEIGHT;
using ftxui::text;
using ftxui::WIDTH;

board::board(std::pair<player, player>* players, int button_size) :
    button_size_(button_size), players_(players) {
    for (unsigned cnt{}; cnt < buttons_.size(); ++cnt) {
        buttons_[cnt].resize(buttons_.size());
    }
}

void board::update(std::function<void()> const& exit) {
    for (unsigned col{}; col < buttons_.size(); ++col) {
        for (unsigned row{}; row < buttons_.size(); ++row) {
            // FIXME:
            // Selector/active button not working properly when puts a char
            auto& cell   = board_[col][row];
            auto& button = buttons_[col][row];

            // FIXME:
            ///////////////////////////////////////////////////////////////////
            // When set button to the active it's blinking/selector resetting
            // and wrong button is active.
            ///////////////////////////////////////////////////////////////////
            if (cell == " ") {
                button = Button(
                    &cell,
                    [this, &cell, exit] {
                        cell = players_->first.get_symbol();
                        exit();
                    },
                    button_style_default(button_size_)
                );
            } else {
                button =
                    Button(&cell, [] {}, button_style_active(button_size_));
            }
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

    // exit();
}

board::buttons_2d& board::get_buttons() { return buttons_; }

std::array<ftxui::Component, 3>& board::get_button_rows() {
    return button_rows_;
}

ftxui::ButtonOption board::button_style_default(int size) {
    ButtonOption option;
    option.label = " ";
    // option.on_click = on_click;

    option.animated_colors.background.Set(
        ftxui::Color::Default, ftxui::Color::RGBA(64, 64, 64, 200)
    );
    option.animated_colors.foreground.Set(ftxui::Color::Red, ftxui::Color::Red);

    option.transform = [size](EntryState const& es) {
        auto element = text(es.label) | ftxui::border |
            color(ftxui::Color::Green) | ftxui::size(WIDTH, EQUAL, size) |
            ftxui::size(ftxui::HEIGHT, EQUAL, size);

        if (es.active) { element |= ftxui::bold; }
        if (es.focused) { element |= color(ftxui::Color::Red); }

        return element;
    };

    return option;
}

ftxui::ButtonOption board::button_style_active(int size) {
    ButtonOption option;
    option.animated_colors.background.Set(
        ftxui::Color::Default, ftxui::Color::RGBA(64, 64, 64, 200)
    );
    option.animated_colors.foreground.Set(
        ftxui::Color::Default, players_->first.get_color()
    );
    option.transform = [size, this](EntryState const& es) {
        auto element = text(es.label) | ftxui::center |
            color(players_->first.get_color()) | border |
            color(players_->first.get_color()) |
            ftxui::size(WIDTH, EQUAL, size) | ftxui::size(HEIGHT, EQUAL, size);

        if (es.active) { element |= ftxui::bold; }
        if (es.focused) { element |= color(ftxui::Color::Green); }

        return element | color(ftxui::Color::GrayDark);
    };

    return option;
}
