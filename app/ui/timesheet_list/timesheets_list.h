//
// Created by Oleksandr Shumihin on
// 25/3/26.
//

#pragma once
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include "./list_item.h"
#include "core/utils/time_format.h"
#include "ui/elements/padding.h"
#include "ui/elements/separator.h"
#include "ui/elements/text.h"
#include "ui/styles.h"
#include "ui/utils.h"

using namespace ftxui;

inline Component TimesheetList(const Timesheet& timesheet, AppState& app_state) {
    static Box scroll_box;

    auto component = Renderer([&timesheet, &app_state](bool focused) {
        Elements rows;

        int index = 0;
        int64_t last_day_ts = 0;
        const auto& items = timesheet.get_items();

        for (const TimesheetItem& item: items) {
            if (last_day_ts != item.day_ts) {
                rows.push_back(
                    vbox({PaddingV(),
                          hbox({
                              Text(format_day_from_utc_day_key(item.day_ts), theme::purple) | bold,
                              filler(),
                              Text(format_sec_to_hhmm(timesheet.get_day_duration(item.day_ts)), theme::purple) | bold,
                              PaddingH(),
                          }),
                          hbox({vbox({Separator(theme::border)}) | flex})}));
                last_day_ts = item.day_ts;
            }

            const bool is_selected = index == app_state.ui.selected_task_index;

            bool is_active = false;
            if (app_state.active_session) {
                is_active = app_state.active_session->task_id == item.task_entity->task.id &&
                    item.day_ts == to_day_key(app_state.active_session->started_at);
            }

            int64_t duration_sec = item.duration_total_sec;

            if (is_active) {
                duration_sec += app_state.active_session->duration_sec;
            }

            rows.emplace_back(ListItem(
                {item.task_entity->task.title, duration_sec, item.task_entity->task.status, is_selected, is_active}));

            ++index;
        }

        return vbox(std::move(rows)) | size(WIDTH, EQUAL, 50) | yframe | vscroll_indicator | reflect(scroll_box) | flex;
    });

    component |= CatchEvent([&](Event event) {
        if (app_state.ui.show_modal) {
            return false;
        }

        if (event == Event::ArrowDown) {
            if (app_state.ui.selected_task_index + 1 < timesheet.get_items().size()) {
                app_state.ui.selected_task_index++;
            }
            return true;
        }
        if (event == Event::ArrowUp) {
            if (app_state.ui.selected_task_index > 0) {
                app_state.ui.selected_task_index--;
            }
            return true;
        }


        if (event.is_mouse()) {
            const auto& mouse = event.mouse();

            if (!is_mouse_inside_box(mouse, scroll_box)) {
                return false;
            }

            if (mouse.button == Mouse::WheelDown) {
                if (app_state.ui.selected_task_index + 1 < timesheet.get_items().size()) {
                    app_state.ui.selected_task_index++;
                }
                return true;
            }

            if (mouse.button == Mouse::WheelUp) {
                if (app_state.ui.selected_task_index > 0) {
                    app_state.ui.selected_task_index--;
                }
                return true;
            }
        }

        return false;
    });

    return component;
}
