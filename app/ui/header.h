//
// Created by Oleksandr Shumihin on 24/3/26.
//

#pragma once
#include <ftxui/dom/elements.hpp>

#include "core/app/models.h"
#include "core/utils/time_format.h"
#include "elements/separator.h"
#include "elements/text.h"
#include "styles.h"
#include "elements/padding.h"

using namespace ftxui;

inline Element Header(const AppState &app_state) {
    long long duration = 0;

    if (app_state.active_session) {
        duration = (get_now() - app_state.active_session.value().started_at) / 1000;
    }

    return vbox({
        vbox({
            PaddingV(),
            Text("TaskFlow") | bold | center,
            Separator(),
            hbox({
                Text("Tasks: " + std::to_string(app_state.tasks.size()), theme::muted) | bold,
                text(" "),
                Separator(),
                text(" "),
                app_state.active_session
                    ? text("Active: " + format_sec_to_hhmmss(duration)) | color(theme::cyan)
                    : text(""),
            })
        })
    });
}
