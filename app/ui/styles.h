//
// Created by Oleksandr Shumihin on 26/03/2026.
//
#pragma once
#include <ftxui/dom/elements.hpp>


namespace theme {
// base
const auto bg = ftxui::Color::RGB(44, 49, 66);
const auto panel = ftxui::Color::RGB(52, 58, 79);
const auto panel2 = ftxui::Color::RGB(60, 68, 92);
const auto border = ftxui::Color::RGB(72, 82, 110);
const auto divider = ftxui::Color::RGB(65, 72, 95);

// text
const auto text = ftxui::Color::RGB(220, 226, 236);
const auto muted = ftxui::Color::RGB(150, 165, 190);
const auto subtle = ftxui::Color::RGB(110, 125, 150);

// accents
const auto blue = ftxui::Color::RGB(120, 190, 255);
const auto cyan = ftxui::Color::RGB(140, 230, 255);
const auto green = ftxui::Color::RGB(110, 220, 170);
const auto yellow = ftxui::Color::RGB(255, 200, 120);
const auto red = ftxui::Color::RGB(255, 130, 130);
const auto accent_soft = ftxui::Color::RGB(90, 130, 180);
const auto purple = ftxui::Color::RGB(180, 140, 255);

// selection
const auto selection = ftxui::Color::RGB(65, 80, 110);
const auto focus = ftxui::Color::RGB(80, 100, 140);
} // namespace theme
