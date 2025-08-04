#include "engine.hpp"
// #include "logger.hpp"
#include "ftxui/dom/elements.hpp"
#include "state_machine.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fmt/base.h>
#include <fmt/format.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>
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
using ftxui::Color;
using ftxui::color;
using ftxui::Component;
using ftxui::Element;
using ftxui::Elements;
using ftxui::Event;
using ftxui::filler;
using ftxui::hbox;
using ftxui::hcenter;
using ftxui::Renderer;
using ftxui::ResizableSplitLeft;
using ftxui::separator;
using ftxui::text;
using ftxui::vbox;
using ftxui::Container::Horizontal;
using ftxui::Container::Vertical;

template <typename... Ts>
struct overloaded : Ts... { // NOLINT(altera-*, fuchsia-*)
    using Ts::operator()...;
};

engine::engine() : screen_(ftxui::ScreenInteractive::Fullscreen()) {
    keys_.reserve(500);
}

ftxui::Element
engine::return_center_text(char const* msg, Color col = Color::Blue) {
    return text(msg) | border | bold | hcenter | color(col);
}

// https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/dom/vbox_hbox.cpp
Element engine::return_center_vbox(Component& component) {
    return vbox({
        hbox({
            filler(),
            component->Render(),
            filler(),
        }),
    });
}

void engine::input_name() {
    auto input_name =
        ftxui::Input(&player_.get_username(), "click here to write: ");

    input_name |= CatchEvent([this](Event const& ev) {
        // Max size of input 12 characters, should not contain
        // '\n' character
        if (s_keyboard(ev)) {
            screen_.ExitLoopClosure()();
            return true;
        }
        return (ev.is_character() && player_.get_username().size() >= 12) ||
            ev.character().contains('\n');
    });


    auto button = Vertical({Button(
        // FIXME: Expand text for the entire button
        labels_[std::to_underlying(label_idx::SAVE)],
        [this] {
            if (!player_.get_username().empty()) {
                screen_.ExitLoopClosure()();
            } else {
                display_warning("Username can't be empty.");
            }
        },
        ButtonOption::Animated(Color::Blue)
    )});

    auto component = Vertical({button, input_name});

    auto renderer = Renderer(component, [this, &input_name, &button] {
        return vbox({
            hbox({text("username: ") | border | bold, input_name->Render()}),
            separator(),
            filler(),
            hbox({text("your username is: " + player_.get_username()) | border}
            ),
            separator(),
            vbox({
                filler(),
                button->Render(),
                filler(),
            }),
        });
    });


    // LOG(player_.get_username());

    screen_.Loop(renderer);
}

bool engine::s_keyboard_menu(ftxui::Event const& ev) {
    if (ev == Event::Escape || ev == Event::Character('q')) {
        state_ = state::exit{};
        return true;
    }
    return false;
}

bool engine::s_keyboard_play(ftxui::Event const& ev) {
    if (ev == Event::Escape || ev == Event::Character('q')) {
        state_ = state::menu{};
        screen_.ExitLoopClosure()();
        return true;
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
    if (state_ != state::play{}) { screen_.ExitLoopClosure()(); }

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
            [this, &ev](state::menu) { return s_keyboard_menu(ev); },
            [this, &ev](state::play) { return s_keyboard_play(ev); },
            [this, &ev](state::exit) { return s_keyboard_exit(ev); },
        },
        state_.get_variant()
    );
}

bool engine::check_flag(flag_render fl) const {
    return (state_flag_ & fl) != 0U;
}

void engine::display_warning(char const* msg) {
    auto button = Vertical({Button(
        labels_[std::to_underlying(label_idx::OK)], [this] { screen_.Exit(); },
        ButtonOption::Animated(Color::Red)
    )});

    auto component = Renderer(button, [&button, &msg]() {
        return vbox({return_center_text(msg), return_center_vbox(button)});
    });

    screen_.Loop(component);
}


void engine::s_render_state() {
    // FIXME:
    if (check_flag(flag_render::STATE)) {
        auto state_str{fmt::format("STATE: {}", fmt::to_string(state_.str_v()))
        };

        for (std::size_t idx{}; idx < state_str.size(); ++idx) {
            auto& p = screen_.PixelAt(
                screen_.dimx() - static_cast<int>(std::ssize(state_str)),
                static_cast<int>(idx) + 1
            );
            p.character        = state_str[idx];
            p.foreground_color = Color::Red;
            p.bold             = true;
        }
    }
}

void engine::s_render() { s_render_state(); }

std::string engine::get_code(Event const& ev) {
    std::string codes{};

    for (auto const obj : ev.input()) {
        codes += ' ' + fmt::to_string(static_cast<unsigned>(obj));
    }

    return codes;
}

void engine::menu_about() {
    // TODO:
    auto button = Horizontal({Button(
        labels_[std::to_underlying(label_idx::BACK)],
        [this]() { screen_.Exit(); }, ButtonOption::Animated(Color::GrayDark)
    )});

    auto component = Renderer(button, [&button]() {
        return vbox(
            {text("Not implemented yet.") | border | bold | hcenter |
                 color(Color::Red),
             return_center_vbox(button)}
        );
    });

    screen_.Loop(component);
}


void engine::menu() {
    using enum label_idx;
    auto buttons = Vertical(
        // PLAY Button
        {Button(
             labels_[std::to_underlying(PLAY)],
             [this]() {
                 state_ = state::play{};
                 screen_.Exit();
             },
             ButtonOption::Animated(Color::GrayLight)
         ),
         // ABOUT Button
         Button(
             labels_[std::to_underlying(ABOUT)],
             [this]() {
                 // TODO:
                 menu_about();
                 screen_.Exit();
             },
             ButtonOption::Animated(Color::GrayDark)
         ),
         // EXIT Button
         Button(
             labels_[std::to_underlying(label_idx::EXIT)],
             [this]() {
                 state_ = state::exit{};
                 screen_.Exit();
             },
             ButtonOption::Animated(Color::GrayDark)
         )}
    );

    auto component = Renderer(buttons, [&buttons]() {
        return vbox({return_center_vbox(buttons)});
    });

    component |= CatchEvent([this](Event const& ev) {
        if (s_keyboard(ev)) {
            screen_.ExitLoopClosure()();
            return true;
        }
        return false;
    });

    screen_.Loop(component);
}

void engine::play() {
    if (player_.get_username().empty()) { input_name(); }

    auto left_column = Renderer([this] {
        Elements children{text("Codes"), separator()};
        auto     num_keys{static_cast<std::size_t>(
            std::max(0, static_cast<int>(keys_.size()) - 20)
        )};
        for (std::size_t idx{num_keys}; idx < keys_.size(); ++idx) {
            children.emplace_back(ftxui::text(get_code(keys_[idx])));
        }
        return ftxui::vbox(children);
    });

    auto right_column = Renderer([this] {
        Elements children{text("Event"), separator()};
        auto     num_keys{static_cast<std::size_t>(
            std::max(0, static_cast<int>(keys_.size()) - 20)
        )};

        for (std::size_t idx{num_keys}; idx < keys_.size(); ++idx) {
            children.emplace_back(text(keys_[idx].DebugString()));
        }
        return vbox(children);
    });


    int  split_size{40};
    auto component = ResizableSplitLeft(left_column, right_column, &split_size);

    component |= border;

    component |= CatchEvent([this](Event const& ev) { return s_keyboard(ev); });

    // TODO: Should be there loop ?...
    screen_.Loop(component);
}

void engine::ask_exit() {
    auto buttons = Horizontal(
        // YES Button
        {Button(
             labels_[std::to_underlying(label_idx::YES)],
             [this]() {
                 quit();
                 return true;
             },
             ButtonOption::Animated(Color::White)
         ),
         // NO Button
         Button(
             labels_[std::to_underlying(label_idx::NO)],
             [this]() {
                 // FIXME: Not working properly 'NO' button
                 state_ = state_.get_previous_variant();
                 screen_.ExitLoopClosure()();
             },
             ButtonOption::Animated(Color::White)
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

    screen_.Loop(component);

    // LOG("EXIT");
}

void engine::quit() {
    running_ = false;
    state_   = state::exit{};
    screen_.ExitLoopClosure()();
}

void engine::run() {
    while (running_) {
        // LOG("size_of_keys: " + fmt::to_string(keys_.size()));

        std::visit(
            overloaded{
                [this](state::menu) { menu(); },
                [this](state::play) { play(); },
                [this](state::exit) { ask_exit(); },
            },
            state_.get_variant()
        );
        s_render();
    }
}
