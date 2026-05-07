#pragma once
#include "core/utils/utils.h"
#include "ui/elements/padding.h"
#include "ui/elements/separator.h"
#include "ui/elements/text.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <thread>

#include "core/controllers/modals_controller.h"
#include "core/utils/report.h"
#include "ui/utils.h"

using namespace ftxui;


inline void refresh_elements(const AppState& app_state, Elements& elements) {
    int64_t prev_day_key = 0;

    for (const auto& item: app_state.timesheet.get_items()) {
        if (prev_day_key != item.day_ts) {
            if (prev_day_key != 0) {
                elements.push_back(PaddingV());
            }
            elements.push_back(hbox({text(format_day_from_utc_day_key(item.day_ts)),
                                     text(" | "),
                                     text(format_sec_to_hhmm(app_state.timesheet.get_day_duration(item.day_ts)))}) |
                               color(theme::purple) | bold);
        }
        elements.push_back(paragraph(" - " + item.task_entity->task.title) | color(theme::text));
        prev_day_key = item.day_ts;
    }
}

inline Component
ReportModal(const AppState& app_state, const ModalsController& modals_controller, std::function<void()>&& on_cancel) {
    static ScrollState scroll_state;
    static std::atomic copied = false;
    static std::string report;
    static EffectState effect_state;

    const auto cancel_btn = Button("Cancel", on_cancel, ButtonOption::Ascii());
    const auto copy_btn = Button(
        "Copy",
        [] {
            copy_to_clipboard_mac(report);
            copied.store(true, std::memory_order_relaxed);
            std::thread([] {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                copied.store(false, std::memory_order_relaxed);
            }).detach();
        },
        ButtonOption::Ascii());


    const auto container = Container::Horizontal({
        copy_btn,
        cancel_btn,
    });


    static Elements elements;

    auto component = Renderer(container, [cancel_btn, copy_btn, &app_state, &modals_controller]() {
        report = make_report(app_state);


        use_effect(
            effect_state,
            [&app_state] {
                elements.clear();
                scroll_state.reset_offset();
                refresh_elements(app_state, elements);
            },
            modals_controller.triggered_value);


        return hbox({filler(),
                     vbox({Text("Report") | bold | center,
                           Separator(theme::accent_soft),
                           vbox({
                               elements,
                           }) | focusPositionRelative(0.f, scroll_state.offset_y) |
                               yframe | vscroll_indicator | reflect(scroll_state.scroll_box) | flex,
                           Separator(theme::accent_soft),
                           hbox({
                               cancel_btn->Render() | center,
                               copy_btn->Render() | (copied ? color(theme::muted) : color(theme::green)),
                           }) | center}) |
                         borderStyled(ROUNDED, theme::accent_soft) | size(WIDTH, EQUAL, 90) |
                         size(HEIGHT, LESS_THAN, 30) | clear_under,
                     filler()});
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
