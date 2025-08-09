#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <array>
#include <exception>
#include <filesystem>
#include <fmt/base.h>
#include <ftxui/screen/color.hpp>
#include <stdexcept>
#include <string>
#include <string_view>

class config {
public:
    explicit config(std::filesystem::path const& path);
    config(config const&)            = delete;
    config(config&&)                 = delete;
    config& operator=(config const&) = delete;
    config& operator=(config&&)      = delete;
    ~config()                        = default;

    void init();
    void parse_data(std::string_view key, std::string_view data);
    void generate_default();

    [[nodiscard]] auto get_default_data() const { return default_data_; }

    [[nodiscard]] std::string_view get_username() {
        try {
            if (username_.empty()) {
                throw std::runtime_error("Empty username.");
            }
            return username_;
        } catch (std::exception const& ex) {
            fmt::println(stderr, "{}", ex.what());
            generate_default();
        }
        return default_username_;
    }

    ftxui::Color get_color() {
        try {
            if (color_ == "White") { return ftxui::Color::White; }
            if (color_ == "Green") { return ftxui::Color::Green; }
            if (color_ == "Blue") { return ftxui::Color::Blue; }

            throw std::runtime_error("Invalid color");
        } catch (std::exception const& ex) {
            fmt::println(stderr, "{}", ex.what());
            generate_default();
        }
        return default_color_;
    }

    [[nodiscard]] char get_symbol() const { return symbol_; }

    void replace(
        std::string_view key, std::string_view source,
        std::string_view destination
    );


    [[nodiscard]] bool was_generated() const { return was_generated_; }

private:
    std::filesystem::path           file_path_;
    std::string                     username_;
    std::string                     color_;
    char                            symbol_;
    bool                            was_generated_;
    std::string_view                default_username_{"username: Player"};
    ftxui::Color                    default_color_{ftxui::Color::Blue};
    std::string_view                default_color_str_v_{"color: Blue"};
    std::string_view                default_symbol_{"symbol: X"};
    std::array<std::string_view, 3> default_data_{
        default_username_, default_color_str_v_, default_symbol_
    };
};

#endif
