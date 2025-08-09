#include "player.hpp"

#include <ftxui/screen/color.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

player::player(std::string_view name, ftxui::Color color, char symbol) :
    username_(name), color_(color), state_(NONE), symbol_(symbol) {}

std::string_view player::get_username() const { return username_; }

std::string& player::get_username() { return username_; }

ftxui::Color const& player::get_color() const { return color_; }

void player::set_color(ftxui::Color color) { color_ = color; }

player::player_state player::state() const { return state_; }

char player::get_symbol() const { return symbol_; }

[[nodiscard]] std::string_view player::get_symbol_str_v() const {
    switch (symbol_) {
        case 'X': return "X";
        case 'O': return "O";
        default: throw std::runtime_error("Unreachable symbol case");
    }
}

void player::set_symbol(char ch) { symbol_ = ch; }

void player::set_username(std::string temp) { username_ = std::move(temp); }
