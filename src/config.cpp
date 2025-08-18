#include "config.hpp"
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fmt/base.h>
#include <fmt/format.h>
#include <fstream>
#include <ftxui/screen/color.hpp>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <toml++/impl/parser.hpp>
#include <toml++/toml.hpp>

using namespace std::literals;

config::config(std::filesystem::path const& path) :
    file_path_(path), symbol_("X") {
    init();
    std::ios_base::sync_with_stdio(false);

    try {
        tbl_ = toml::parse_file(path.c_str());
        parse_data();
    } catch (std::exception const& err) {
        fmt::println(stderr, "{}", err.what());
        generate_default();
    }
}

// Initializing default config.ini
void config::init() {
    if (!std::filesystem::exists(file_path_)) {
        generate_default();
    } else {
        was_generated_ = true;
    }
}

void config::parse_data() {
    std::string_view const username =
        tbl_["tictactoe"]["username"].value_or(""sv);
    username_ = username;

    std::string_view const color = tbl_["tictactoe"]["color"].value_or(""sv);
    color_                       = color;

    std::string_view const symbol = tbl_["tictactoe"]["symbol"].value_or(""sv);
    symbol_                       = symbol;
}

void config::generate_default() {
    file_.open(file_path_, std::ios::out);

    if (!file_.is_open()) {
        throw std::runtime_error(
            fmt::format("Can't open {}", file_path_.string())
        );
    }

    tbl_ = toml::parse(default_toml, file_path_.string());

    file_ << tbl_ << '\n';
    std::cout << tbl_ << '\n';

    file_.close();

    fmt::println(stderr, "Generated default configuration.toml");
}

void config::replace(std::string_view key, std::string_view value) {
    auto* str = tbl_["tictactoe"][key].as_string();
    *str      = value;

    file_.open(file_path_, std::ios::out);

    if (!file_.is_open()) {
        throw std::runtime_error(
            fmt::format("Can't open {}", file_path_.string())
        );
    }

    file_ << tbl_ << '\n';

    file_.close();
}

toml::table& config::get_config() { return tbl_; }

[[nodiscard]] std::string_view config::get_username() {
    try {
        if (username_.empty()) { throw std::runtime_error("Empty username."); }
        return username_;
    } catch (std::exception const& ex) {
        fmt::println(stderr, "{}", ex.what());
        generate_default();
    }
    return default_username;
}

ftxui::Color config::get_color() {
    try {
        if (color_ == "White") { return ftxui::Color::White; }
        if (color_ == "Green") { return ftxui::Color::Green; }
        if (color_ == "Blue") { return ftxui::Color::Blue; }

        throw std::runtime_error("Invalid color");
    } catch (std::exception const& ex) {
        fmt::println(stderr, "{}", ex.what());
        fmt::println("Only supported colors are: White, Green, Blue");
        generate_default();
    }
    return default_color_;
}

[[nodiscard]] std::string_view config::get_symbol() const { return symbol_; }

[[nodiscard]] bool config::was_generated() const { return was_generated_; }
