#include "options.hpp"
#include <fmt/format.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <functional>
#include <string>
#include <utility>
#include <vector>
#include "box_utils.hpp"
#include "label_buttons.hpp"
#include "player.hpp"

using ftxui::ButtonOption;
using ftxui::center;
using ftxui::Color;
using ftxui::Component;
using ftxui::EntryState;
using ftxui::GREATER_THAN;
using ftxui::HEIGHT;
using ftxui::text;
using ftxui::WIDTH;
using ftxui::Container::Vertical;

options::options(options::players& players) :
    players_(players), default_button_option_(button_style()),
    selector_(players_.first.get_symbol() == "X" ? 0 : 1),
    temp_(players_.first.get_username_str_v()) {

    // Toggle symbol
    toggle_symbol_ = Vertical({toggles_});
}

ftxui::ButtonOption options::button_style(int width, int height) {
    ButtonOption option = ButtonOption::Simple();
    // button_option_.on_click  = on_click;
    option.animated_colors.background.Set(Color::Black, Color::Blue);
    option.animated_colors.foreground.Set(Color::Black, Color::Blue);
    option.transform = [width, height](EntryState const& es) {
        auto element = text(es.label) | center |
            ftxui::size(WIDTH, GREATER_THAN, width) |
            ftxui::size(HEIGHT, GREATER_THAN, height) | color(Color::Blue);

        if (es.focused) { element |= color(Color::Green); }

        return element | center;
    };
    return option;
}

void options::input_name_events(std::function<void()> exit) {
    temp_       = players_.first.get_username_str_v();
    input_name_ = ftxui::Input(
        &temp_, "click here to write: ", ftxui::InputOption::Spacious()
    );
    input_name_ |= ftxui::CatchEvent([this, &exit](ftxui::Event const& ev) {
        if (ev == ftxui::Event::Escape) { exit(); }
        // Max size of input 12 characters
        return (ev.is_character() && temp_.size() >= 12) ||
            ev == ftxui::Event::Return;
    });
}

void options::display_warning(char const* msg) {
    auto screen      = ftxui::ScreenInteractive::Fullscreen();
    auto back_button = Vertical({Button(
        labels[fmt::underlying(Label::BACK)], [&screen] { screen.Exit(); },
        button_style(25, 5)
    )});

    auto renderer = Renderer(back_button, [&back_button, msg]() {
        return ftxui::vbox(
            {make_center_text(msg), make_center_vbox(back_button)}
        );
    });

    screen.Loop(renderer);
}


ftxui::Component options::get_input_name() { return input_name_; }

ftxui::ButtonOption options::get_button_option() {
    return default_button_option_;
}

ftxui::Component options::get_toggle_symbol() { return toggle_symbol_; }

std::vector<std::string> const& options::get_toggle_entries() {
    return toggle_entries_;
}

int& options::get_selector() { return selector_; }

Component options::get_save_button(std::function<void()> const& exit) {
    auto save_button = ftxui::Container::Vertical({Button(
        labels[fmt::underlying(Label::SAVE)],
        [exit, this]() {
            if (!temp_.empty()) {
                players_.first.set_username(std::move(temp_));
                exit();
            } else {
                display_warning("Username can't be empty.");
            }
        },
        default_button_option_
    )});


    return save_button;
}

std::string const& options::get_temp_str() { return temp_; }
