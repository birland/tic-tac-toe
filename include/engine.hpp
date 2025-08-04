#ifndef ENGINE_HPP
#define ENGINE_HPP
#include "state_machine.hpp"

#include <array>
#include <cstdint>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
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

    static ftxui::Element return_center_vbox(ftxui::Component& component);
    void                  s_keyboard();
    [[nodiscard]] bool    check_flag(flag_render fl) const;
    void                  s_render_state();
    void                  s_render();
    static std::string    get_code(ftxui::Event const& ev);
    void                  menu_about();
    void                  menu();
    void                  play();
    void                  exit();
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
        SIZE
    };
    static constexpr std::array<char const*, 6> labels_{
        "        PLAY        ", "        ABOUT        ",
        "        EXIT        ", "        YES        ",
        "        NO        ",   "        BACK        "
    };
    static_assert(labels_.size() == std::to_underlying(label_idx::SIZE));
};

#endif
