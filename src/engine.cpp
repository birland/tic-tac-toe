#include "engine.hpp"
#include <array>
#include <cassert>
#include <chrono>
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
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/canvas.hpp>
#include <ftxui/dom/deprecated.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/box.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/pixel.hpp>
#include <ftxui/screen/screen.hpp>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <variant>
#include <vector>
#include "board.hpp"
#include "box_utils.hpp"
#include "label_buttons.hpp"
#include "player.hpp"
#include "state_machine.hpp"

using ftxui::bold;
using ftxui::border;
using ftxui::Button;
using ftxui::ButtonOption;
using ftxui::CatchEvent;
using ftxui::center;
using ftxui::Color;
using ftxui::color;
using ftxui::Component;
using ftxui::Element;
using ftxui::Elements;
using ftxui::Event;
using ftxui::filler;
using ftxui::flex;
using ftxui::hbox;
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

engine::engine() :
    main_screen_(ftxui::ScreenInteractive::Fullscreen()), config_("config.ini"),
    players_(
        {std::make_unique<player>(player(
             config_.get_username(), config_.get_color(), config_.get_symbol()
         )),
         std::make_unique<player>(player(
             "Enemy", Color::Red, config_.get_symbol() == 'X' ? 'O' : 'X'
         ))}
    ),
    options_(&config_, {players_.first.get(), players_.second.get()}),
    board_({players_.first.get(), players_.second.get()}) {
    keys_.reserve(500);
}

ftxui::ButtonOption
engine::button_style(int size /* std::function<void()> on_click*/) {
    // auto option = ButtonOption::Animated();
    auto option = ButtonOption::Simple();
    // option.on_click  = on_click;
    option.animated_colors.background.Set(Color::GrayDark, Color::White);
    option.animated_colors.foreground.Set(Color::White, Color::Green);
    option.transform = [size](ftxui::EntryState const& es) {
        auto element = text(es.label) | center | bold |
            ftxui::size(ftxui::WIDTH, ftxui::EQUAL, size) |
            ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, size / 3);

        if (es.focused) { element |= color(ftxui::Color::Green); }

        return element;
    };

    return option;
}

// Input name, toggle set symbol
void engine::menu_options() {
    auto screen = ftxui::ScreenInteractive::Fullscreen();
    options_.input_name_events(screen.ExitLoopClosure());

    auto input  = options_.get_input_name();
    auto toggle = options_.get_toggle_symbol();
    auto button = options_.get_save_button(screen.ExitLoopClosure());

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
                {text(
                     "your current username is: " +
                     players_.first->get_username()
                 ) |
                 border | bold}
            ),
            hbox(
                {text("your new username is: " + options_.get_temp_str()) |
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

    screen.Loop(renderer);

    // Save new name to the config file
    std::string_view const old_username = config_.get_username();
    std::string_view const new_username = players_.first->get_username();
    config_.replace("username", old_username, new_username);

    // Save new symbol to the config file
    std::string_view const old_symbol = players_.first->get_symbol_str_v();
    std::string_view const new_symbol = options_.get_toggle_entries(
    )[static_cast<std::size_t>(options_.get_selector())];

    players_.first->set_symbol(new_symbol);
    players_.second->set_symbol(old_symbol);

    config_.replace("symbol", old_symbol, new_symbol);
}

bool engine::s_keyboard_menu(ftxui::Event const& ev) {
    if (ev == Event::Escape || ev == Event::Character('q')) {
        state_ = state::exit{};
        main_screen_.Exit();
        return true;
    }
    return false;
}

bool engine::s_keyboard_play(ftxui::Event const& ev) {
    if (ev == Event::Escape || ev == Event::Character('q')) {
        state_ = state::menu{};
        s_reset_game();
        main_screen_.Exit();
        return true;
    }

    if (ev == Event::r) {
        s_reset_game();
        main_screen_.Exit();
    }

    if (ev == Event::F1) {
        state_ = state::menu{};
    } else if (ev == Event::F2) {
        state_ = state::play{};
    } else if (ev == Event::F3) {
        state_ = state::exit{};
    }
    if (state_ != state::play{}) {
        main_screen_.Exit();
        return true;
    }

    return false;
}

bool engine::s_keyboard_exit(ftxui::Event const& ev) {
    if (ev == Event::Escape || ev == Event::Character('q')) {
        quit();
        return true;
    }
    return false;
}

bool engine::s_keyboard(Event const& ev) {
    keys_.emplace_back(ev);

    return std::visit(
        overloaded{
            [&](state::menu) { return s_keyboard_menu(ev); },
            [&](state::play) { return s_keyboard_play(ev); },
            [&](state::exit) { return s_keyboard_exit(ev); },
        },
        state_.get_variant()
    );
}

void engine::s_reset_game() { s_create_game(); }

void engine::s_create_game() {
    // players_.first = std::make_unique<player>(player(
    //     config_.get_username(), config_.get_color(), config_.get_symbol()
    // ));

    // char second_player_symbol{'X'};
    // if (players_.first->get_symbol() == 'X') { second_player_symbol = 'O'; }

    // players_.second = std::make_unique<player>(
    //     player("Enemy", Color::Red, second_player_symbol)
    // );

    board_ = board({players_.first.get(), players_.second.get()});
}

std::string engine::get_code(Event const& ev) {
    std::string codes{};

    for (auto const obj : ev.input()) {
        codes += ' ' + fmt::to_string(static_cast<unsigned>(obj));
    }

    return codes;
}

ftxui::Component
engine::end_game(char const* label, Color color, ftxui::Component& buttons) {
    auto component = Renderer(buttons, [&buttons, color, label] {
        return vbox(
            return_center_text(label, color), return_center_vbox(buttons)
        );
    });

    return component;
}

ftxui::Component engine::game_result_buttons(std::function<void()> const& exit
) {
    constexpr int button_size = 12;
    auto          buttons     = Horizontal(
        {Button(
             "AGAIN",
             [this, exit] {
                 s_reset_game();
                 exit();
             },
             button_style(button_size)
         ),
                      Button(
             "EXIT",
             [this, exit] {
                 s_reset_game();
                 state_ = state::menu{};
                 exit();
                 stop_signal_ = true;
             },
             button_style(button_size)
         )}
    );

    return buttons;
}

void engine::show_game_result(player::state_variant st) {
    // auto screen = ftxui::ScreenInteractive::Fullscreen();
    auto buttons = game_result_buttons(main_screen_.ExitLoopClosure());
    ftxui::Component component;

    std::visit(
        overloaded{
            [&component, &buttons](player::state::won) {
                component =
                    end_game("      YOU WON!        ", Color::Green, buttons);
            },
            [&buttons, &component](player::state::losed) {
                component =
                    end_game("      YOU LOSED!      ", Color::Red, buttons);
            },
            [&buttons, &component](player::state::draw) {
                component =
                    end_game("        DRAW!         ", Color::Yellow3, buttons);
            },
        },
        st
    );

    // screen.Loop(component);
    main_screen_.Loop(component);
}

void engine::s_update_logic() {
    if (board_.is_end() && board_.get_timer().elapsed_seconds() >= 1.0) {
        auto var = board_.check_victory();
        show_game_result(var);
    }
}

void engine::s_update() {
    board_.update_draw();
    board_.update_moves();
    s_update_logic();
}

void engine::menu_about(int button_size) {
    auto button = Horizontal({Button(
        labels_[std::to_underlying(label_idx::BACK)],
        [this]() { main_screen_.ExitLoopClosure()(); },
        button_style(button_size)
    )});

    auto component = Renderer(button, [&button]() {
        return vbox(
            {text("Not implemented yet.") | border | bold | center |
                 color(Color::Red),
             return_center_vbox(button)}
        );
    });

    main_screen_.Loop(component);
}

void engine::menu() {
    constexpr int button_size{15};
    using enum label_idx;
    auto buttons = Vertical(
        // PLAY Button
        {Button(
             labels_[std::to_underlying(PLAY)],
             [this]() {
                 state_ = state::play{};
                 main_screen_.ExitLoopClosure()();
             },
             button_style(button_size)
         ),
         // ABOUT Button
         Button(
             labels_[std::to_underlying(ABOUT)],
             [this]() { menu_about(button_size); }, button_style(button_size)

         ),
         // OPTIONS Button
         Button(
             labels_[std::to_underlying(OPTIONS)], [this] { menu_options(); },
             button_style(button_size)
         ),
         // EXIT Button
         Button(
             labels_[std::to_underlying(EXIT)],
             [this]() {
                 state_ = state::exit{};
                 main_screen_.Exit();
             },
             button_style(button_size)
         )}
    );

    auto component = Renderer(buttons, [&buttons]() {
        return vbox(return_center_vbox(buttons));
    });

    component |= CatchEvent([this](Event const& ev) { return s_keyboard(ev); });

    main_screen_.Loop(component);
}

void engine::play() {
    // Ask user to input name only on the first launch
    // when config is not generated yet.
    if (players_.first->get_username().empty() || !config_.was_generated()) {
        menu_options();
    }
    s_create_game();

    stop_signal_ = false;

    board_.update_draw();

    auto& rows{board_.get_button_rows()};

    // Flex for every row to expand them on the entire window
    auto layout = Vertical({rows[0] | flex, rows[1] | flex, rows[2] | flex});

    auto const&    window_color = Color::Green;
    ftxui::Element window_label;

    auto renderer = Renderer(layout, [&] {
        auto* player = board_.get_player_turn();
        if (player->get_username() == players_.first->get_username()) {
            window_label = text("MOVE: " + player->get_username()) | center |
                color(player->get_color());
        } else {
            window_label =
                text(
                    "MOVE: " + player->get_username() + " in " +
                    fmt::to_string(board_.get_secs_to_move()) + " seconds"
                ) |
                center | color(player->get_color());
        }

        return vbox(
            ftxui::window(
                window_label | center, layout->Render(),
                ftxui::BorderStyle::DASHED
            ) |
            color(window_color) | flex
        );
    });


    renderer |= CatchEvent([this](Event const& ev) { return s_keyboard(ev); });

    ftxui::Loop loop{&main_screen_, renderer};

    while (!loop.HasQuitted() && (!stop_signal_)) {
        loop.RunOnce();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        s_update();
    }
}

// TODO(?):
// Implement modal dialogue
// https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/component/modal_dialog.cpp
void engine::ask_exit() {
    auto          screen = ftxui::ScreenInteractive::Fullscreen();
    constexpr int button_size{10};
    auto          buttons = Horizontal(
        // YES Button
        {Button(
             labels_[std::to_underlying(label_idx::YES)],
             [this, &screen]() {
                 quit();
                 screen.ExitLoopClosure()();
                 return true;
             },
             button_style(button_size)
         ),
                  // NO Button
                  Button(
             labels_[std::to_underlying(label_idx::NO)],
             [this, &screen]() {
                 state_ = state_.get_previous_variant();
                 screen.ExitLoopClosure()();
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
        return s_keyboard(ev);
    });

    screen.Loop(component);
}

void engine::quit() {
    running_ = false;
    state_   = state::exit{};
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
