#include "engine.hpp"
#include "board.hpp"
#include "box_utils.hpp"
#include "label_buttons.hpp"
#include "player.hpp"
#include "state_machine.hpp"

#include <array>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fmt/base.h>
#include <fmt/format.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/canvas.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/box.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/pixel.hpp>
#include <ftxui/screen/screen.hpp>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

using ftxui::bold;
using ftxui::border;
using ftxui::Button;
using ftxui::ButtonOption;
using ftxui::CatchEvent;
using ftxui::center;
using ftxui::Checkbox;
using ftxui::Color;
using ftxui::color;
using ftxui::Component;
using ftxui::Element;
using ftxui::Elements;
using ftxui::Event;
using ftxui::filler;
using ftxui::flex;
using ftxui::hbox;
using ftxui::Radiobox;
using ftxui::Renderer;
using ftxui::separator;
using ftxui::text;
using ftxui::vbox;
using ftxui::Container::Horizontal;
using ftxui::Container::Vertical;

template <typename... Ts>
struct overloaded : Ts... { // NOLINT(altera-*, fuchsia-*)
    using Ts::operator()...;
};

constexpr unsigned max_button_size{9};

engine::engine() :
    screen_(ftxui::ScreenInteractive::Fullscreen()), config_("config.ini"),
    players_(
        {player(
             config_.get_username(), config_.get_color(), config_.get_symbol()
         ),
         player("Enemy", Color::Red, config_.get_symbol() == 'X' ? 'O' : 'X')}
    ),
    options_(&config_, &screen_, &players_),
    board_(max_button_size, &players_) {
    keys_.reserve(500);
    s_create_game();
}

ftxui::ButtonOption
engine::button_style(int size /* std::function<void()> on_click*/) {
    // auto option = ButtonOption::Animated();
    auto option = ButtonOption::Simple();
    // option.on_click  = on_click;
    option.animated_colors.background.Set(Color::GrayDark, Color::White);
    option.animated_colors.foreground.Set(Color::White, Color::Green);
    option.transform = [size](ftxui::EntryState const& es) {
        auto element = text(es.label) | ftxui::center | bold |
            ftxui::size(ftxui::WIDTH, ftxui::EQUAL, size) |
            ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, size / 3);

        if (es.focused) { element |= color(ftxui::Color::Green); }

        return element;
    };

    return option;
}

// Input name, toggle set symbol
void engine::input() {
    options_.input_name_events();

    auto input  = options_.get_input_name();
    auto toggle = options_.get_toggle_symbol();
    auto button = options_.get_save_button(screen_);

    auto containers = Vertical({
        input,
        toggle,
        button,
    });

    auto renderer = Renderer(containers, [&, this] {
        return vbox({
            hbox(
                {text("username: ") | border | bold,
                 options_.get_input_name()->Render()}
            ),
            separator(),
            vbox(text("Symbol: ") | bold | flex, toggle->Render() | border),
            separator(),
            filler(),
            hbox(
                {text("your username is: " + players_.first.get_username()) |
                 border | bold}
            ),
            separator(),
            vbox({
                filler(),
                button->Render(),
                filler(),
            }),
        });
    });

    auto component = CatchEvent(renderer, [this](auto const& ev) {
        if (ev == Event::Return) {
            screen_.ExitLoopClosure()();
            return true;
        }
        return ev == Event::Return;
    });


    screen_.Loop(component);

    // Save new name to the config file
    config_.replace(config_.get_username(), players_.first.get_username());

    // Save new symbol to the config file
    config_.replace(
        players_.first.get_symbol_str_v(),
        options_.get_toggle_entries(
        )[static_cast<std::size_t>(options_.get_selector())]
    );
}

bool engine::s_keyboard_menu(
    ftxui::Event const& ev, std::function<void()> const& exit
) {
    if (ev == Event::Escape || ev == Event::Character('q')) {
        state_ = state::exit{};
        exit();
        return true;
    }
    return false;
}

bool engine::s_keyboard_play(
    ftxui::Event const& ev, std::function<void()> const& exit
) {
    if (ev == Event::Escape || ev == Event::Character('q')) {
        state_ = state::menu{};
        exit();
        return true;
    }

    if (ev == Event::r) {
        //
        s_reset_game();
        exit();
    }

    if (ev == Event::F1) {
        state_ = state::menu{};
    } else if (ev == Event::F2) {
        state_ = state::play{};
    } else if (ev == Event::F3) {
        state_ = state::exit{};
    } else if (ev == Event::F12) {
        state_flag_ ^= flag_render::STATE;
    }
    if (state_ != state::play{}) {
        exit();
        return true;
    }

    return false;
}

bool engine::s_keyboard_exit(
    ftxui::Event const& ev, std::function<void()> const& exit
) {
    if (ev == Event::Escape || ev == Event::Character('q')) {
        quit(exit);
        return true;
    }
    return false;
}

bool engine::s_keyboard(Event const& ev, std::function<void()> const& exit) {
    keys_.emplace_back(ev);

    return std::visit(
        overloaded{
            [&](state::menu) { return s_keyboard_menu(ev, exit); },
            [&](state::play) { return s_keyboard_play(ev, exit); },
            [&](state::exit) { return s_keyboard_exit(ev, exit); },
        },
        state_.get_variant()
    );
}

bool engine::check_flag(flag_render fl) const {
    return (state_flag_ & fl) != 0U;
}


// TODO:
// https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/component/gallery.cpp
Component engine::component_debug() const {
    std::vector<std::string> entries{"DEBUG_INFO"};
    int                      debug_menu_selected{};
    bool                     debug_menu = check_flag(flag_render::DEBUG_INFO);

    auto layout = Vertical(
        {Checkbox("DEBUG_MENU", &debug_menu),
         Radiobox(&entries, &debug_menu_selected) | border |
             ftxui::Maybe(&debug_menu),

         Renderer([] { return text("Test") | color(Color::Red); }) |
             ftxui::Maybe([&] { return debug_menu_selected == 1; })}
    );


    // return ftxui::Maybe(layout, &debug_menu);
    return layout;
}

void engine::s_reset_game() { s_create_game(); }

void engine::s_create_game() {
    board_ = board(max_button_size, &players_);

    players_.first = player(
        config_.get_username(), config_.get_color(), config_.get_symbol()
    );

    char symbol{'X'};
    if (players_.first.get_symbol() == 'X') { symbol = 'O'; }

    players_.second = player("Enemy", Color::Red, symbol);
}

std::string engine::get_code(Event const& ev) {
    std::string codes{};

    for (auto const obj : ev.input()) {
        codes += ' ' + fmt::to_string(static_cast<unsigned>(obj));
    }

    return codes;
}

void engine::game_logic(auto& window_color) {
    // TODO:
    window_color = Color::Green;
    switch (players_.first.state()) {
        case player::WON:
        case player::LOSED:
        case player::DRAW:
        case player::NONE: break;
        default: throw std::runtime_error("Unreachable player state.");
    }
}

void engine::menu_about(int button_size) {
    auto button = Horizontal({Button(
        labels_[std::to_underlying(label_idx::BACK)],
        [this]() { screen_.ExitLoopClosure()(); }, button_style(button_size)
    )});

    auto component = Renderer(button, [&button]() {
        return vbox(
            {text("Not implemented yet.") | border | bold | center |
                 color(Color::Red),
             return_center_vbox(button)}
        );
    });

    screen_.Loop(component);
}

void engine::menu() {
    auto button_size{15};
    using enum label_idx;
    auto buttons = Vertical(
        // PLAY Button
        {Button(
             labels_[std::to_underlying(PLAY)],
             [this]() {
                 state_ = state::play{};
                 screen_.ExitLoopClosure()();
             },
             button_style(button_size)
         ),
         // ABOUT Button
         Button(
             labels_[std::to_underlying(ABOUT)],
             [this, button_size]() {
                 menu_about(button_size);
                 screen_.ExitLoopClosure()();
             },
             button_style(button_size)

         ),
         // OPTIONS Button
         Button(
             labels_[std::to_underlying(OPTIONS)], [this] { input(); },
             button_style(button_size)
         ),
         // EXIT Button
         Button(
             labels_[std::to_underlying(EXIT)],
             [this]() {
                 state_ = state::exit{};
                 screen_.ExitLoopClosure()();
             },
             button_style(button_size)
         )}
    );

    auto component = Renderer(buttons, [&buttons]() {
        return vbox(return_center_vbox(buttons));
    });

    component |= CatchEvent([this](Event const& ev) {
        return s_keyboard(ev, screen_.ExitLoopClosure());
    });

    screen_.Loop(component);
}

void engine::play() {
    // Asking user to input name only on the first launch
    // when config is not generated yet.
    if (players_.first.get_username().empty() || !config_.was_generated()) {
        input();
    }

    board_.update(screen_.ExitLoopClosure());

    auto rows{board_.get_button_rows()};

    auto layout = Vertical({rows[0], rows[1], rows[2]});


    auto c = ftxui::Canvas(100, 100);
    // std::vector<ftxui::Canvas> canvases{};
    // for (unsigned idx{}; idx < max_button_size; ++idx) {
    //     canvases.emplace_back(100, 100);
    //     LOG(fmt::to_string(buttons[idx]->Index()));
    //     canvases[idx].DrawText(
    //         0, 0, fmt::to_string(buttons[idx]->Index()), player_.get_color()
    //     );
    // }
    c.DrawText(0, 0, "X", [this](ftxui::Pixel& p) {
        p.foreground_color = players_.first.get_color();
    });
    // c.DrawBlockCircle(30, 30, 10);
    // c.DrawBlockCircle(0, 0, 100);

    auto doc = canvas(&c) | center | border | center;

    auto window_color = Color::Green;
    game_logic(window_color);

    auto renderer = Renderer(layout, [&] {
        return vbox(
            ftxui::window(
                text(players_.first.get_username()) |
                    color(players_.first.get_color()),
                layout->Render(), ftxui::BorderStyle::DASHED
            ) | color(window_color),
            doc
        );
    });

    renderer |= CatchEvent([this](Event const& ev) {
        return s_keyboard(ev, screen_.ExitLoopClosure());
    });

    screen_.Loop(renderer);
}

// TODO:
// Implement modal dialogue
// https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/component/modal_dialog.cpp
void engine::ask_exit() {
    int  button_size{10};
    auto buttons = Horizontal(
        // YES Button
        {Button(
             labels_[std::to_underlying(label_idx::YES)],
             [this]() {
                 quit(screen_.ExitLoopClosure());
                 return true;
             },
             button_style(button_size)
         ),
         // NO Button
         Button(
             labels_[std::to_underlying(label_idx::NO)],
             [this]() {
                 state_ = state_.get_previous_variant();
                 screen_.ExitLoopClosure()();
             },
             button_style(button_size)
         )}
    );

    auto renderer = Renderer(buttons, [&buttons]() {
        return vbox(
            {return_center_text("Do you want to exit?"),
             return_center_vbox(buttons)}
        );
    });

    auto component = CatchEvent(renderer, [this](Event const& ev) {
        return s_keyboard(ev, screen_.ExitLoopClosure());
    });

    screen_.Loop(component);
}

void engine::quit(std::function<void()> const& exit) {
    running_ = false;
    state_   = state::exit{};
    exit();
}

void engine::run() {
    while (running_) {
        std::visit(
            overloaded{
                [this](state::menu) { menu(); },
                [this](state::play) { play(); },
                [this](state::exit) { ask_exit(); },
            },
            state_.get_variant()
        );
    }
}
