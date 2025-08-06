#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <array>
#include <filesystem>
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

    void               init();
    void               parse_data(std::string_view key, std::string_view data);
    [[nodiscard]] auto get_default_data() const { return default_data_; }
    [[nodiscard]] auto get_username() const { return username_; }
    [[nodiscard]] auto get_color() const {
        if (color_ == "White") { return ftxui::Color::White; }
        if (color_ == "Green") { return ftxui::Color::Green; }
        if (color_ == "Blue") { return ftxui::Color::Blue; }

        throw std::runtime_error("Invalid color");
    }
    [[nodiscard]] char get_symbol() const { return symbol_; }

    void replace(std::string_view source, std::string_view destination);


    [[nodiscard]] bool was_generated() const { return was_generated_; }

private:
    std::filesystem::path           file_path_;
    std::string                     username_;
    std::string                     color_;
    char                            symbol_;
    bool                            was_generated_;
    std::string_view                default_username_{"username: Player"};
    std::string_view                default_color_{"color: Blue"};
    std::string_view                default_symbol_{"symbol: X"};
    std::array<std::string_view, 3> default_data_{
        default_username_, default_color_, default_symbol_
    };
};

#endif
