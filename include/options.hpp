#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <functional>
#include <string>
#include <utility>
#include <vector>
#include "config.hpp"
#include "player.hpp"

class options {
public:
    explicit options(config* config, std::pair<player, player>* players);
    options(options const&)            = delete;
    options(options&&)                 = delete;
    options& operator=(options const&) = delete;
    options& operator=(options&&)      = delete;
    ~options()                         = default;

    void update();

    [[nodiscard]] static ftxui::ButtonOption
                                   button_style(int width = 5, int height = 5);
    [[nodiscard]] ftxui::Component get_input_name();
    [[nodiscard]] ftxui::ButtonOption             get_button_option();
    [[nodiscard]] ftxui::Component                get_toggle_symbol();
    [[nodiscard]] std::vector<std::string> const& get_toggle_entries();
    [[nodiscard]] int&                            get_selector();

    // Save player username when SAVE pressed
    // and return component with save button
    // OR component with display warning when username is empty.
    [[nodiscard]] ftxui::Component get_save_button(std::function<void()> exit);

    [[nodiscard]] std::string const& get_temp_str();

    // Writing data to the temp string to prevent
    // current username corruption
    void input_name_events();

    static void display_warning(char const* msg);

private:
    // Config
    config* config_;
    // Players
    std::pair<player, player>* players_;
    // Components
    ftxui::Component    input_name_;
    ftxui::Component    toggle_symbol_;
    ftxui::ButtonOption default_button_option_;
    // Toggle symbol
    std::vector<std::string> toggle_entries_{"X", "O"};
    int                      selector_{config_->get_symbol() == 'X' ? 0 : 1};
    ftxui::Component toggles_{ftxui::Toggle(&toggle_entries_, &selector_)};
    // Temp string for input
    std::string temp_;
};

#endif
