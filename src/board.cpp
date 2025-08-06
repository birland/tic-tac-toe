#include "board.hpp"
#include "logger.hpp"
#include "player.hpp"

#include <array>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/canvas.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/util/ref.hpp>
#include <functional>
#include <stdexcept>
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

board::board(
    unsigned button_count, std::pair<player, player>* players, int button_size
) : button_count_(button_count), button_size_(button_size), players_(players) {
    buttons_.resize(button_count_);
}

void board::update(std::function<void()> exit) {
    for (unsigned idx{}; idx < buttons_.size(); ++idx) {
        // Set buttons to active when clicked
        // .front() because std::string in array will contain only one character
        if (board_[idx].front() == ' ') {
            buttons_[idx] = Button(
                &board_[idx],
                [this, idx, exit] {
                    board_[idx] = players_->first.get_symbol();
                    exit();
                },
                button_style_default(button_size_, [] {})
            );
        } else {
            buttons_[idx] =
                Button(&board_[idx], [] {}, button_style_active(button_size_));
        }
    }

    button_rows_ = {
        {ftxui::Container::Horizontal(
             {buttons_[0], buttons_[1], buttons_[2]}, &selector_
         ),
         ftxui::Container::Horizontal(
             {buttons_[3], buttons_[4], buttons_[5]}, &selector_
         ),
         ftxui::Container::Horizontal(
             {buttons_[6], buttons_[7], buttons_[8]}, &selector_
         )}
    };

    exit();
}

ftxui::Components& board::get_buttons() { return buttons_; }

std::array<ftxui::Component, 3>& board::get_button_rows() {
    return button_rows_;
}

ftxui::ButtonOption
board::button_style_default(int size, std::function<void()> /*on_click*/) {
    ButtonOption option;
    option.label = " ";
    // option.on_click = on_click;

    option.animated_colors.background.Set(
        ftxui::Color::Default, ftxui::Color::Default
    );
    option.animated_colors.foreground.Set(
        ftxui::Color::Default, ftxui::Color::Default
    );

    option.transform = [size](EntryState const& es) {
        auto element = text(es.label) | ftxui::border |
            color(ftxui::Color::Green) | ftxui::size(WIDTH, EQUAL, size) |
            ftxui::size(ftxui::HEIGHT, EQUAL, size);

        if (es.active) { element |= ftxui::bold; }
        if (es.focused) { element |= ftxui::inverted; }

        return element | color(ftxui::Color::Red);
    };

    return option;
}

ftxui::ButtonOption board::button_style_active(int size) {
    ButtonOption option;
    option.animated_colors.background.Set(
        ftxui::Color::Default, players_->first.get_color()
    );
    option.animated_colors.foreground.Set(
        ftxui::Color::Default, players_->first.get_color()
    );
    option.transform = [size, this](EntryState const& es) {
        auto element = text(es.label) | color(players_->first.get_color()) |
            border | ftxui::size(WIDTH, EQUAL, size) |
            ftxui::size(HEIGHT, EQUAL, size);

        if (es.focused) { element |= color(ftxui::Color::Green); }

        return element | color(ftxui::Color::GrayDark);
    };

    return option;
}
