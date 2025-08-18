#ifndef BOX_UTILS_HPP
#define BOX_UTILS_HPP

#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/color.hpp>
#include <string>

inline ftxui::Element
make_center_text(char const* msg, ftxui::Color col = ftxui::Color::Blue) {
    return ftxui::text(msg) | ftxui::border | ftxui::bold | ftxui::center |
        ftxui::color(col);
}

// https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/dom/vbox_hbox.cpp
inline ftxui::Element make_center_vbox(ftxui::Component const& component) {
    return ftxui::vbox(
        {ftxui::hbox({ftxui::filler(), component->Render(), ftxui::filler()})}
    );
}

#endif
