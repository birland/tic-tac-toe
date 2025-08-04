#include "engine.hpp"
#include "logger.hpp"
#include "state_machine.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fmt/base.h>
#include <fmt/format.h>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>
#include <string>
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
using ftxui::separator;
using ftxui::text;
using ftxui::vbox;
using ftxui::Container::Horizontal;
using ftxui::Container::Vertical;


// TODO:
// 1. Implement config
// 2. Implement 2 languages to choose

engine::engine() : screen_(ftxui::ScreenInteractive::Fullscreen()) {
    keys_.reserve(500);
}

Element engine::return_center_vbox(Component& component) {
    return vbox(
        filler(), hbox(filler(), component->Render(), filler()), filler()
    );
}

void engine::s_keyboard() {
    // TODO:
    // Implement keyboard system to track events
    // from every state
}

bool engine::check_flag(flag_render fl) const {
    return (state_flag_ & fl) != 0U;
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
    auto button    = Horizontal({Button(
        labels_[std::to_underlying(label_idx::BACK)],
        [this]() { screen_.Exit(); }, ButtonOption::Animated(Color::GrayDark)
    )});
    auto component = Renderer(button, [&button]() {
        return vbox(vbox(
            text("Not implemented yet.") | border | bold | hcenter |
                color(Color::Red),
            return_center_vbox(button)
        ));
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
        return vbox(return_center_vbox(buttons));
    });

    screen_.Loop(component);
}

void engine::play() {
    auto left_column = ftxui::Renderer([this] {
        Elements children{ftxui::text("Codes"), ftxui::separator()};
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
    auto component =
        ftxui::ResizableSplitLeft(left_column, right_column, &split_size);

    component |= border;

    component |= CatchEvent([this](Event const& ev) {
        keys_.emplace_back(ev);

        if (ev == Event::Escape || ev == Event::Character('q')) {
            state_ = state::exit{};
        }

        if (ev == Event::F1) { state_ = state::menu{}; }
        if (ev == Event::F2) { state_ = state::play{}; }
        if (ev == Event::F3) { state_ = state::exit{}; }

        if (ev == Event::F12) { state_flag_ ^= flag_render::STATE; }

        if (state_ != state::play{}) { screen_.Exit(); }

        return false;
    });

    // TODO: Should be there loop ?...
    screen_.Loop(component);
}

void engine::exit() {
    auto buttons = Horizontal(
        // YES Button
        {Button(
             labels_[std::to_underlying(label_idx::YES)],
             [this]() {
                 running_ = false;
                 state_   = state::exit{};
                 screen_.Exit();
             },
             ButtonOption::Animated(Color::White)
         ),
         // NO Button
         Button(
             labels_[std::to_underlying(label_idx::NO)],
             [this]() {
                 // FIXME: Not working properly 'NO' button
                 state_ = state_.get_previous_variant();
                 screen_.Exit();
             },
             ButtonOption::Animated(Color::White)
         )}
    );

    auto component = Renderer(buttons, [&buttons]() {
        return vbox(
            vbox(
                text("Do you want to exit?") | border | bold | hcenter |
                color(Color::Blue)
            ),
            filler(), return_center_vbox(buttons), filler()
        );
    });

    screen_.Loop(component);

    LOG("EXIT");
}

template <typename... Ts>
struct overloaded : Ts... { // NOLINT(altera-*, fuchsia-*)
    using Ts::operator()...;
};

#include <fmt/color.h>

void engine::run() {
    while (running_) {
        LOG("size_of_keys: " + fmt::to_string(keys_.size()));

        std::visit(
            overloaded{
                [this](state::menu) { menu(); },
                [this](state::play) { play(); },
                [this](state::exit) { exit(); },
            },
            state_.get_variant()
        );
        s_render();
    }
}
