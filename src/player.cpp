#include "player.hpp"
#include <ftxui/screen/color.hpp>
#include <string>
#include <string_view>
#include <utility>

player::player(
    std::string_view name, ftxui::Color color, std::string_view symbol
) :
    username_(name), symbol_(symbol), prev_symbol_(symbol == "X" ? 'X' : 'O'),
    color_(color), state_() {}

std::string_view    player::get_username_str_v() const { return username_; }
std::string const&  player::get_username() const { return username_; }
ftxui::Color const& player::get_color() const { return color_; }
std::string_view    player::get_symbol() const { return symbol_; }
char                player::get_prev_symbol() const { return prev_symbol_; }
player::state_variant const& player::get_variant() { return variant_; }
struct player::state         player::get_state() const { return state_; }
[[nodiscard]] bool           player::is_won() const { return is_won_; }

void player::set_username(std::string temp) { username_ = std::move(temp); }
void player::set_symbol(std::string_view str_v) {
    if (symbol_ == "X") {
        prev_symbol_ = 'X';
    } else {
        prev_symbol_ = 'O';
    }

    symbol_ = str_v;
}
void player::set_symbol(char ch) {
    if (ch == 'X') {
        symbol_ = "X";
    } else {
        symbol_ = "O";
    }
}
void player::set_color(ftxui::Color color) { color_ = color; }

void player::set_won() { is_won_ = true; }
