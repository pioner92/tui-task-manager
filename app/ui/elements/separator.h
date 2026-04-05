//
// Created by Oleksandr Shumihin on 26/03/2026.
//

#pragma once
#include <ftxui/dom/elements.hpp>

#include "ui/styles.h"

using namespace ftxui;


inline Element Separator(const Color c = theme::border) {
    return separator() | color(c);
}
