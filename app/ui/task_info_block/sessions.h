//
// Created by Oleksandr Shumihin on 01/04/2026.
//

#pragma once
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include "core/storage/models.h"
#include "core/utils/time_format.h"
#include "core/utils/utils.h"
#include "ui/styles.h"
#include "ui/elements/padding.h"
#include "ui/elements/separator.h"

using namespace ftxui;

inline Element Sessions(const std::deque<SessionModel> &task_sessions, const Component &edit_btn) {
    int64_t prev_day_key = 0;
    Elements rows;

    const bool is_empty = task_sessions.empty();

    std::unordered_map<int64_t, int64_t> day_durations;
    day_durations.reserve(3); // 3 days is +- common case

    for (const auto &s: task_sessions) {
        if (s.end_at) {
            int64_t day_key = to_day_key(s.start_at);
            day_durations[day_key] += get_session_duration_sec(s);
        }
    }

    for (const auto &s: task_sessions) {
        const int64_t day_key = to_day_key(s.start_at);

        if (prev_day_key != day_key) {
            prev_day_key = day_key;

            rows.push_back(vbox({
                PaddingV(),
                hbox({
                    hbox({
                        text(" "),
                        text(format_day_from_utc_day_key(day_key)) | bold,
                        text(" | ") | bold,
                        text(format_sec_to_hhmm(day_durations[day_key])) | bold,
                        text(" ")
                    }) |
                    color(theme::purple) | inverted,
                }),
                Separator(theme::panel2)
            }));
        }

        auto ended_at_str = s.end_at.has_value() ? format_ms_to_hhmm_local(s.end_at.value()) : "now";

        rows.push_back(hbox({
            text(" "),
            text(std::format("{}-{}", format_ms_to_hhmm_local(s.start_at), std::move(ended_at_str))) |
            color(theme::muted),
            text(" "),
            s.end_at.has_value()
                ? hbox({text("| "), text(format_sec_to_hhmmss(get_session_duration_sec(s)))}) | color(theme::muted)
                : hbox(),
            PaddingH(),
        }));
    }
    return vbox({
        hbox({
            text("Sessions: ") | color(theme::purple) | bold,
            !is_empty ? edit_btn->Render() : emptyElement(),
        }),
        is_empty ? Separator(theme::panel2) : emptyElement(),
        vbox(std::move(rows))
    });
}
