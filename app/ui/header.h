//
// Created by Oleksandr Shumihin on 24/3/26.
//

#pragma once
#include <ftxui/dom/elements.hpp>

#include "core/app/models.h"
#include "core/utils/time_format.h"
#include "elements/padding.h"
#include "elements/text.h"
#include "styles.h"

using namespace ftxui;


constexpr size_t BUF_SIZE = 32;
constexpr size_t ACTIVE_LABEL_OFFSET = 7;
static char active_title[BUF_SIZE] = "Active ";

constexpr size_t TITLE_OFFSET = 7;
static char title[BUF_SIZE] = "Tasks: ";

inline Element Header(const AppState& app_state) {
    if (app_state.active_session) {
        format_sec_to_hhmmss(
            app_state.active_session->duration_sec, active_title + ACTIVE_LABEL_OFFSET, BUF_SIZE - ACTIVE_LABEL_OFFSET);
    }

    if (!app_state.tasks.empty()) {
        char* start = title + TITLE_OFFSET;
        const auto [p, ec] = std::to_chars(start, title + BUF_SIZE, app_state.tasks.size());

        if (ec == std::errc()) {
            *p = '\0';
        }
    }

    return vbox({vbox({hbox({
        vbox({
            PaddingV(),
            Text(title, theme::muted) | bold,
        }),
        text(" "),
        separator() | color(theme::divider) | size(HEIGHT, EQUAL, 2),
        text(" "),
        app_state.active_session
            ? vbox({PaddingV(),
                    hbox({text("● ") | color(theme::red) | bold, text(active_title) | color(theme::cyan) | bold})})
            : emptyElement(),
    })})});
}
