#ifndef PLAYER_HPP
#define PLAYER_HPP
#include <ftxui/screen/color.hpp>
#include <string>
#include <string_view>
#include <variant>

class player {
public:
    player() = default;
    player(std::string_view name, ftxui::Color color, std::string_view symbol);
    player(player const&)            = delete;
    player(player&&)                 = default;
    player& operator=(player const&) = default;
    player& operator=(player&&)      = default;
    ~player()                        = default;

    struct state {
        struct won {};
        struct losed {};
        struct draw {};
    };

    using state_variant = std::variant<state::won, state::losed, state::draw>;

    [[nodiscard]] std::string_view    get_username_str_v() const;
    [[nodiscard]] std::string const&  get_username() const;
    [[nodiscard]] ftxui::Color const& get_color() const;
    [[nodiscard]] std::string_view    get_symbol() const;
    [[nodiscard]] char                get_prev_symbol() const;
    state_variant const&              get_variant();
    [[nodiscard]] state               get_state() const;
    [[nodiscard]] bool                is_won() const;
    void                              set_username(std::string temp);
    void                              set_symbol(std::string_view str_v);
    void                              set_symbol(char ch);
    void                              set_color(ftxui::Color color);
    void                              set_won();

private:
    std::string                                         username_;
    std::string_view                                    symbol_;
    char                                                prev_symbol_{};
    ftxui::Color                                        color_;
    std::variant<state::won, state::losed, state::draw> variant_;
    struct state                                        state_{};
    bool                                                is_won_{};
};

#endif
