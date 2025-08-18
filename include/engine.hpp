#ifndef ENGINE_HPP
#define ENGINE_HPP
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/screen/color.hpp>
#include <functional>
#include <utility>
#include <vector>
#include "board.hpp"
#include "config.hpp"
#include "options.hpp"
#include "player.hpp"
#include "state_machine.hpp"

class engine {
public:
    engine();
    engine(engine const&)            = delete;
    engine(engine&&)                 = delete;
    engine& operator=(engine const&) = delete;
    engine& operator=(engine&&)      = delete;
    ~engine()                        = default;

    [[nodiscard]] static ftxui::ButtonOption button_style(int size);
    bool s_keyboard_play(ftxui::Event const& ev);
    bool s_keyboard_menu(ftxui::Event const& ev);
    bool s_keyboard_exit(ftxui::Event const& ev);
    bool s_keyboard(ftxui::Event const& ev);
    void s_reset_game();
    void s_create_game();

    [[nodiscard]] static ftxui::Component
    end_game(char const* label, ftxui::Color color, ftxui::Component& buttons);
    [[nodiscard]] ftxui::Component
         game_result_buttons(std::function<void()> const& exit);
    void show_game_result(player::state_variant st);

    void s_update_logic();
    void s_update();

    void menu_about(int button_size);
    void menu_options();
    void menu();
    void play();
    void ask_exit();
    void quit();
    void run();

    [[nodiscard]] state_machine& state() { return state_; }

private:
    // Screen
    ftxui::ScreenInteractive main_screen_;
    // Stop signal for exit from play loop
    bool stop_signal_{false};
    // State
    state_machine state_;
    // Events
    std::vector<ftxui::Event> keys_;
    bool                      running_{true};
    // Config
    config config_;
    // Players
    using players = std::pair<player, player>;
    players players_;
    // Options
    options options_;
    // Board
    board board_;
};

#endif
