//
// Created by Oleksandr Shumihin on 25/3/26.
//

#pragma once
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <utility>

#include "core/app/models.h"
#include "core/controllers/modals_controller.h"
#include "core/storage/models.h"
#include "core/utils/time_format.h"
#include "core/utils/utils.h"
#include "description.h"
#include "sessions.h"
#include "ui/elements/padding.h"
#include "ui/elements/separator.h"
#include "ui/elements/task_status_badge.h"
#include "ui/elements/text.h"
#include "ui/styles.h"
#include "ui/utils.h"

struct TaskEntity;
using namespace ftxui;

inline Element ContainerComponent(Element children) {
    return vbox({PaddingV(), text("Task info") | bold | center, Separator(), std::move(children)}) | flex;
}

inline Component TaskInfoBlock(const AppState& app_state, const ModalsController& modals_controller) {
    const auto& items = app_state.timesheet.get_items();
    static ScrollState scroll_state;

    auto copy_title_btn =
        Button(
            "Copy",
            [&items, &app_state] {
                copy_to_clipboard_mac(items[app_state.ui.selected_task_index].task_entity->task.title);
            },
            ButtonOption::Ascii()) |
        color(theme::focus) | inverted;

    auto copy_description_btn =
        Button(
            "Copy",
            [&app_state, &items] {
                copy_to_clipboard_mac(items[app_state.ui.selected_task_index].task_entity->task.description);
            },
            ButtonOption::Ascii()) |
        color(theme::focus) | inverted;

    auto edit_sessions_button = Button(
                                    "Edit",
                                    [&modals_controller] {
                                        modals_controller.open_edit_sessions();
                                    },
                                    ButtonOption::Ascii()) |
        color(theme::focus) | inverted;

    const auto layer = Container::Vertical({
        edit_sessions_button,
        copy_title_btn,
        copy_description_btn,
    });

    auto component =
        Renderer(layer, [&app_state, &items, copy_title_btn, copy_description_btn, edit_sessions_button]() {
            if (items.empty()) {
                return ContainerComponent(emptyElement());
            }

            const auto& timesheet_item = items[app_state.ui.selected_task_index];
            const auto& task_sessions = timesheet_item.task_entity->sessions;
            const auto& task = timesheet_item.task_entity->task;

            // Reset offset on task change
            useEffect(
                [] {
                    scroll_state.reset_offset();
                },
                task.id);

            auto content = vbox({
                               hbox({
                                   TaskStatusBadge(task.status),
                                   filler(),
                                   vbox({
                                       Text("created: " + format_ts2(task.created_at), theme::subtle),
                                       task.created_at != task.updated_at
                                           ? Text("updated: " + format_ts2(task.updated_at), theme::subtle)
                                           : text(""),
                                   }),
                                   PaddingH(),
                               }),
                               PaddingV(),
                               hbox({
                                   copy_title_btn->Render(),
                               }),
                               PaddingV(),
                               paragraph(task.title),
                               Separator(theme::panel2),
                               PaddingV(),
                               Description(task.description, copy_description_btn),
                               PaddingV(),
                               Sessions(task_sessions, edit_sessions_button),
                           }) |
                focusPositionRelative(0.f, scroll_state.offset_y) | yframe | vscroll_indicator |
                reflect(scroll_state.scroll_box) | flex;

            return ContainerComponent(std::move(content));
        });

    component |= CatchEvent([&](Event event) {
        if (event.is_mouse()) {
            const auto& mouse = event.mouse();
            scroll_state.handle_mouse(mouse);
        }

        return false;
    });
    return component;
}
