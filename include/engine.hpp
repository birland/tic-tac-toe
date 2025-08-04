#ifndef ENGINE_HPP
#define ENGINE_HPP
#include "player.hpp"
#include "state_machine.hpp"

#include <array>
#include <cstdint>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>
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

    static ftxui::Element return_center_text(char const* msg, ftxui::Color);
    static ftxui::Element return_center_vbox(ftxui::Component& component);
    [[nodiscard]] bool    check_flag(flag_render fl) const;
    void                  display_warning(char const* msg);
    void                  input_name();
    bool                  s_keyboard_menu(ftxui::Event const& ev);
    bool                  s_keyboard_play(ftxui::Event const& ev);
    bool                  s_keyboard_exit(ftxui::Event const& ev);
    bool                  s_keyboard(ftxui::Event const& ev);
    void                  s_render_state();
    void                  s_render_usernames();
    void                  s_render();
    static std::string    get_code(ftxui::Event const& ev);
    void                  menu_about();
    void                  menu();
    void                  play();
    void                  ask_exit();
    void                  quit();
    void                  run();


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
    // Text
    enum class label_idx : std::uint8_t {
        PLAY,
        ABOUT,
        EXIT,
        YES,
        NO,
        BACK,
        SAVE,
        OK,
        SIZE
    };
    static constexpr std::array<char const*, 8> labels_{
        "        PLAY        ", "        ABOUT        ", "        EXIT        ",
        "        YES        ",  "        NO        ",    "        BACK        ",
        "        SAVE        ", "        OK        "
    };
    static_assert(labels_.size() == std::to_underlying(label_idx::SIZE));

    // Player
    player player_;
};

#endif
