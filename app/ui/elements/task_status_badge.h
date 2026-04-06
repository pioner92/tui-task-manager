//
// Created by Oleksandr Shumihin on 26/03/2026.
//

#pragma once
#include <ftxui/dom/elements.hpp>

#include "core/storage/models.h"
#include "core/utils/utils.h"
#include "ui/styles.h"

using namespace ftxui;


inline Element TaskStatusBadge(const TaskStatus status) {
    static Color table[3] = {theme::status::todo, theme::status::in_progress, theme::status::done};

    return hbox({
               filler(),
               text(STATUS_TEXT_TABLE[std::to_underlying(status)]),
               filler(),
           }) |
        border | color(table[std::to_underlying(status)]) | size(WIDTH, EQUAL, 15);
}


inline Element TaskStatusBadgeSmall(const TaskStatus status) {
    static Color table[3] = {theme::status::todo, theme::status::in_progress, theme::status::done};

    return text(STATUS_TEXT_SHORT_TABLE[std::to_underlying(status)]) | italic |
        color(table[std::to_underlying(status)]);
}
