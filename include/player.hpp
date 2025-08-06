#ifndef PLAYER_HPP
#define PLAYER_HPP
#include <cstdint>
#include <ftxui/screen/color.hpp>
#include <string>
#include <string_view>

class player {
public:
    player() = default;
    player(std::string_view name, ftxui::Color color, char symbol);
    player(player const&)            = delete;
    player(player&&)                 = default;
    player& operator=(player const&) = default;
    player& operator=(player&&)      = default;
    ~player()                        = default;

    enum player_state : std::uint8_t { WON, LOSED, DRAW, NONE };

    [[nodiscard]] std::string_view    get_username() const;
    std::string&                      get_username();
    [[nodiscard]] ftxui::Color const& get_color() const;
    void                              set_color(ftxui::Color color);
    [[nodiscard]] char                get_symbol() const;
    [[nodiscard]] std::string_view    get_symbol_str_v() const;
    void                              set_symbol(char ch);

    [[nodiscard]] player_state state() const;

private:
    std::string  name_;
    ftxui::Color color_;
    player_state state_{};
    char         symbol_{};
};

#endif
