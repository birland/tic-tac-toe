#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <filesystem>
#include <fstream>
#include <ftxui/screen/color.hpp>
#include <string_view>
#include <toml++/impl/forward_declarations.hpp>
#include <toml++/toml.hpp> // NOLINT

class config {
public:
    explicit config(std::filesystem::path const& path);
    config(config const&)            = delete;
    config(config&&)                 = delete;
    config& operator=(config const&) = delete;
    config& operator=(config&&)      = delete;
    ~config()                        = default;

    void init();
    void parse_data();
    void generate_default();
    void replace(std::string_view key, std::string_view value);

    toml::table&                   get_config();
    [[nodiscard]] auto             get_default_data() const;
    [[nodiscard]] std::string_view get_username();
    ftxui::Color                   get_color();
    [[nodiscard]] std::string_view get_symbol() const;
    [[nodiscard]] bool             is_generated();
    [[nodiscard]] bool             is_first_launch() const;
    [[nodiscard]] bool&            is_first_launch();

private:
    std::fstream          file_;
    toml::table           tbl_;
    std::filesystem::path file_path_;
    std::string_view      username_;
    std::string_view      color_;
    std::string_view      symbol_;
    ftxui::Color          default_color_{ftxui::Color::Blue};
    using sv = std::string_view;
    static constexpr std::string_view default_toml{sv(R"(
        [tictactoe]
        color = "Blue"
        symbol = "X"
        username = "Player"
        )")};
    static constexpr std::string_view default_color_str_v{sv("Blue")};
    static constexpr std::string_view default_symbol{sv("X")};
    static constexpr std::string_view default_username{sv("Player")};
    bool                              is_first_launch_{};
};

#endif
