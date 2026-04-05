//
// Created by Oleksandr Shumihin on 26/03/2026.
//

#pragma once
#include <ftxui/dom/elements.hpp>

#include "ui/styles.h"

using namespace ftxui;


inline Element Text(const std::string_view title, const Color cl = theme::text) {
    return text(title) | color(cl);
}
