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

using ftxui::GREATER_THAN;
using ftxui::text;
using ftxui::Container::Vertical;
using std::to_underlying;

options::options(
    config* const config, ftxui::ScreenInteractive* const screen,
    std::pair<player, player>* const players
) :
    config_(config), screen_(screen), players_(players),
    selector_(config_->get_symbol() == 'X' ? 0 : 1) {
    // ButtonOption initialization
    button_option_ = ftxui::ButtonOption();
    auto size      = 5;
    // button_option_.on_click  = on_click;
    button_option_.animated_colors.background.Set(
        ftxui::Color::Black, ftxui::Color::Blue
    );
    button_option_.animated_colors.foreground.Set(
        ftxui::Color::Black, ftxui::Color::Blue
    );
    button_option_.transform = [size](ftxui::EntryState const& es) {
        auto element = text(es.label) | color(ftxui::Color::Green) |
            ftxui::bold | ftxui::center |
            ftxui::size(ftxui::HEIGHT, GREATER_THAN, size);

        if (es.focused) { element |= color(ftxui::Color::Green); }

        return element | color(ftxui::Color::Blue) | ftxui::center;
    };

    // Toggle symbol
    toggle_symbol_ = Vertical({toggles_});
}

void options::update() { selector_ = config_->get_symbol() == 'X' ? 0 : 1; }


ftxui::Component options::get_input_name() { return input_name_; }

ftxui::ButtonOption options::get_button_option() { return button_option_; }

ftxui::Component options::get_toggle_symbol() { return toggle_symbol_; }

std::vector<std::string> const& options::get_toggle_entries() {
    return toggle_entries_;
}

int& options::get_selector() { return selector_; }

ftxui::Component options::get_save_button(ftxui::ScreenInteractive& screen) {
    save_button_ = ftxui::Container::Vertical({Button(
        labels_[std::to_underlying(label_idx::SAVE)],
        [&, this]() {
            if (!players_->first.get_username().empty()) {
                screen.ExitLoopClosure()();
            } else {
                display_warning("Username can't be empty.");
            }
        },
        button_option_
    )});

    return save_button_;
}

void options::input_name_events() {
    input_name_ =
        ftxui::Input(&players_->first.get_username(), "click here to write: ");
    input_name_ |= ftxui::CatchEvent([this](ftxui::Event const& ev) {
        // Max size of input 12 characters, should not contain
        // '\n' character
        return (ev.is_character() && players_->first.get_username().size() >= 12
               ) ||
            ev == ftxui::Event::Return;
    });
}

void options::display_warning(char const* msg) {
    auto button = Vertical({Button(
        labels_[to_underlying(label_idx::OK)],
        [this] { screen_->ExitLoopClosure(); },
        ftxui::ButtonOption::Animated(ftxui::Color::Red)
    )});

    auto component = Renderer(button, [&button, &msg]() {
        return ftxui::vbox({return_center_text(msg), return_center_vbox(button)}
        );
    });

    screen_->Loop(component);
}
