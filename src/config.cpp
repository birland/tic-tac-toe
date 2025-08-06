#include "config.hpp"
#include <cstdio>
#include <cstring>
#include <filesystem>
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
        throw std::runtime_error(fmt::format("Can't open {}", path.c_str()));
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
        std::ofstream file{file_path_, std::ios::binary | std::ios::out};
        if (!file.is_open()) {
            throw std::runtime_error(
                fmt::format("Can't open {}", file_path_.c_str())
            );
        }

        for (auto str_v : default_data_) { file << str_v << '\n'; }

        file.close();
    } else {
        was_generated_ = true;
    }
}

void config::parse_data(std::string_view key, string_view data) {
    if (key == "username") {
        username_ = data;
    } else if (key == "color") {
        color_ = data;
    } else if (key == "symbol") {
        symbol_ = data.front();
    }
}

void config::replace(std::string_view source, std::string_view destination) {
    auto old_file_path = (file_path_.string() + ".temp");

    auto err = std::rename(file_path_.c_str(), old_file_path.c_str());

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
            fmt::format("Can't open: {}", file_path_.c_str())
        );
    }

    std::string            line;
    [[maybe_unused]] auto* buffer = old_file.rdbuf();
    while (std::getline(old_file, line, '\n')) {
        auto pos = line.find(source);
        if (pos != std::string::npos) {
            line.replace(pos, source.size(), destination);
            new_file << line << '\n';
            continue;
        }
        new_file << line << '\n';
    }

    old_file.close();
    new_file.close();

    auto rem_err = std::remove(old_file_path.c_str());
    if (rem_err != 0) {
        throw std::runtime_error(
            std::string("Can't remove old file") + std::strerror(rem_err)
        );
    }
}
