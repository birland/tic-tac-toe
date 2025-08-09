#include "options.hpp"
#include "box_utils.hpp"
#include "config.hpp"
#include "label_buttons.hpp"
#include "player.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>
#include <functional>
#include <string>
#include <utility>
#include <vector>

using ftxui::ButtonOption;
using ftxui::center;
using ftxui::Color;
using ftxui::EntryState;
using ftxui::EQUAL;
using ftxui::HEIGHT;
using ftxui::text;
using ftxui::WIDTH;
using ftxui::Container::Vertical;
using std::to_underlying;

options::options(
    config* const config, ftxui::ScreenInteractive* const screen,
    std::pair<player, player>* const players
) :
    config_(config), screen_(screen), players_(players),
    default_button_option_(button_style()),
    selector_(config_->get_symbol() == 'X' ? 0 : 1),
    temp_(players_->first.get_username()) {

    // Toggle symbol
    toggle_symbol_ = Vertical({toggles_});
}

void options::update() { selector_ = config_->get_symbol() == 'X' ? 0 : 1; }

ftxui::ButtonOption options::button_style(int width, int height) {
    ButtonOption option = ButtonOption::Simple();
    // button_option_.on_click  = on_click;
    option.animated_colors.background.Set(Color::Black, Color::Blue);
    option.animated_colors.foreground.Set(Color::Black, Color::Blue);
    option.transform = [width, height](EntryState const& es) {
        auto element = text(es.label) | center |
            ftxui::size(WIDTH, EQUAL, width) |
            ftxui::size(HEIGHT, ftxui::EQUAL, height);

        if (es.focused) { element |= color(Color::Green); }

        return element | center;
    };
    return option;
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

ftxui::Component options::get_save_button(std::function<void()> exit) {
    save_button_ = ftxui::Container::Vertical({Button(
        labels_[std::to_underlying(label_idx::SAVE)],
        [&, this]() {
            if (!temp_.empty()) {
                players_->first.set_username(temp_);
                exit();
            } else {
                display_warning("Username can't be empty.", exit);
            }
        },
        default_button_option_
    )});

    return save_button_;
}

std::string const& options::get_temp_str() { return temp_; }


void options::input_name_events() {
    input_name_ = ftxui::Input(
        &temp_, "click here to write: ", ftxui::InputOption::Spacious()
    );
    input_name_ |= ftxui::CatchEvent([this](ftxui::Event const& ev) {
        // Max size of input 12 characters
        return (ev.is_character() && temp_.size() >= 12) ||
            ev == ftxui::Event::Return;
    });
}

void options::display_warning(char const* msg, std::function<void()> exit) {
    auto button = Vertical({Button(
        labels_[to_underlying(label_idx::BACK)], [&exit] { exit(); },
        button_style(25, 2)
    )});

    auto component = Renderer(button, [&button, msg]() {
        return ftxui::vbox({return_center_text(msg), return_center_vbox(button)}
        );
    });

    screen_->Loop(component);
}
