#ifndef PLAYER_HPP
#define PLAYER_HPP
#include <ftxui/screen/color.hpp>
#include <string>
#include <string_view>
#include <variant>

class player {
public:
    player() = default;
    player(std::string_view name, ftxui::Color color, char symbol);
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

    [[nodiscard]] std::string_view    get_username() const;
    std::string&                      get_username();
    [[nodiscard]] ftxui::Color const& get_color() const;
    void                              set_color(ftxui::Color color);
    [[nodiscard]] char                get_symbol() const;
    [[nodiscard]] std::string_view    get_symbol_str_v() const;
    void                              set_symbol(char ch);
    void                              set_symbol(std::string_view str_v);
    // Moves string
    void set_username(std::string temp);

    state_variant const& get_variant();
    [[nodiscard]] state  get_state() const;

private:
    std::string                                         username_;
    ftxui::Color                                        color_;
    std::variant<state::won, state::losed, state::draw> variant_;
    struct state                                        state_{};
    char                                                symbol_{};
};

#endif
