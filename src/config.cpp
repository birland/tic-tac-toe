#include "config.hpp"
#include <array>
#include <cstdio>
#include <cstring> // for strerror_s
#include <exception>
#include <filesystem>
#include <fmt/base.h>
#include <fmt/format.h>
#include <fstream>
#include <ios>
#include <istream>
#include <stdexcept>
#include <string>
#include <string_view>

using std::string_view;

config::config(std::filesystem::path const& path) :
    file_path_(path), symbol_('X'), was_generated_(false) {
    init();

    std::ifstream file{path, std::ios::binary | std::ios::in};

    if (!file.is_open()) {
        throw std::runtime_error(fmt::format("Can't open {}", path.string()));
    }

    std::string line;
    std::string token;

    while (std::getline(file, line, ':') &&
           std::getline(file >> std::ws, token, '\n')) {
        parse_data(line, token);
    }

    file.close();
}

// Initializing default config.ini
void config::init() {
    if (!std::filesystem::exists(file_path_)) {
        generate_default();
    } else {
        was_generated_ = true;
    }
}

void config::parse_data(std::string_view key, string_view data) {
    try {
        if (data.empty()) { throw std::runtime_error("Empty data"); }

        if (key == "username") {
            username_ = data;
        } else if (key == "color") {
            color_ = data;
        } else if (key == "symbol") {
            symbol_ = data.front();
        } else {
            throw std::runtime_error("Corrupted config.ini");
        }
    } catch (std::exception const& ex) {
        fmt::println(stderr, "{}", ex.what());
        generate_default();
    }
}
void config::generate_default() {
    fmt::println(stderr, "Generated default config.");

    if (std::filesystem::exists(file_path_)) {
        auto err = std::remove(file_path_.string().c_str());
        if (err != 0) { throw std::runtime_error("Failed to remove file."); }
    }

    std::ofstream file{file_path_, std::ios::binary | std::ios::out};
    if (!file.is_open()) {
        throw std::runtime_error(
            fmt::format("Can't open {}", file_path_.string())
        );
    }

    for (auto str_v : default_data_) { file << str_v << '\n'; }

    file.close();
}

void config::replace(
    std::string_view key, std::string_view source, std::string_view destination
) {
    auto old_file_path = (file_path_.string() + ".temp");

    auto err = std::rename(file_path_.string().c_str(), old_file_path.c_str());

    if (err != 0) { throw std::runtime_error("Failed to rename."); }

    // Open renamed file to read data and copy to new file with destination
    // string
    std::ifstream old_file{old_file_path};
    std::ofstream new_file{file_path_};

    if (!old_file.is_open()) {
        throw std::runtime_error(
            fmt::format("Can't open: {}", old_file_path.c_str())
        );
    }

    if (!new_file.is_open()) {
        throw std::runtime_error(
            fmt::format("Can't open: {}", file_path_.string())
        );
    }

    std::string line;
    std::string token;
    while (std::getline(old_file >> std::ws, line, ':') &&
           std::getline(old_file >> std::ws, token)) {
        if (key == line) {
            auto pos = token.find(source);
            if (pos != std::string::npos) {
                token.replace(pos, token.size(), destination);
                line += ": " + token;
                new_file << line << '\n';
                continue;
            }
        }
        line += ": " + token;
        new_file << line << '\n';
    }

    old_file.close();
    new_file.close();

    auto rem_err = std::remove(old_file_path.c_str());

    if (rem_err != 0) {
        std::array<char, 255> buffer{};
        (void)strerror_s(buffer.data(), buffer.size(), rem_err);
        throw std::runtime_error(
            "Can't remove old file" + std::string(buffer.begin(), buffer.end())
        );
    }
}
