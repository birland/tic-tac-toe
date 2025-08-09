#ifndef ENGINE_HPP
#define ENGINE_HPP
#include "board.hpp"
#include "config.hpp"
#include "options.hpp"
#include "player.hpp"
#include "state_machine.hpp"

#include <cstdint>
#include <ftxui/component/component.hpp>
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

class engine {
public:
    engine();
    engine(engine const&)            = delete;
    engine(engine&&)                 = delete;
    engine& operator=(engine const&) = delete;
    engine& operator=(engine&&)      = delete;
    ~engine()                        = default;

    enum flag_render : std::uint8_t {
        STATE      = 1U << 0U,
        DEBUG_INFO = 1U << 1U // TODO: Implement
    };

    static ftxui::ButtonOption button_style(int size);
    [[nodiscard]] bool         check_flag(flag_render fl) const;
    void                       display_warning(char const* msg);
    void                       input();
    bool
    s_keyboard_menu(ftxui::Event const& ev, std::function<void()> const& exit);
    bool
    s_keyboard_play(ftxui::Event const& ev, std::function<void()> const& exit);
    bool
    s_keyboard_exit(ftxui::Event const& ev, std::function<void()> const& exit);
    bool s_keyboard(ftxui::Event const& ev, std::function<void()> exit);
    [[nodiscard]] ftxui::Component component_debug() const;
    void                           s_render_usernames();
    void                           s_reset_game();
    void                           s_create_game();

    static std::string get_code(ftxui::Event const& ev);
    void               game_logic(auto& window_color);

    void menu_about(int button_size);
    void menu();
    void play();
    void ask_exit();
    void quit(std::function<void()> const& exit);
    void run();


    [[nodiscard]] state_machine& state() { return state_; }

private:
    // Screen
    ftxui::ScreenInteractive screen_;
    // State
    state_machine state_;
    std::uint8_t  state_default_flags_{STATE | DEBUG_INFO};
    std::uint8_t  state_flag_{state_default_flags_};
    // Events
    std::vector<ftxui::Event> keys_;
    bool                      running_{true};
    // Config
    config config_;
    // Players
    std::pair<player, player> players_;
    // Options
    options options_;
    // Board
    board board_;
};

#endif
