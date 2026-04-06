#pragma once
#include "core/controllers/modals_controller.h"
#include "core/utils/time_format.h"
#include "ui/elements/padding.h"
#include "ui/elements/separator.h"
#include "ui/elements/text.h"
#include "ui/styles.h"
#include "ui/utils.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>


inline Component EditSessionsModal(const ModalsController& modals_controller,
                                   std::function<void()>&& on_confirm,
                                   std::function<void()>&& on_cancel) {
    using namespace ftxui;
    Components session_checkboxes;

    auto sessions_container = Container::Vertical({});
    static EffectState effect_state;

    auto rebuild_list = [&modals_controller, sessions_container] {
        sessions_container->DetachAllChildren();

        if (modals_controller.edit_sessions_state.task_entity) {
            int64_t prev_day = 0;
            for (const auto& s: modals_controller.edit_sessions_state.task_entity.value()->sessions) {
                int64_t day_key = to_day_key(s.start_at);

                const std::string label = format_ms_to_hhmm_local(s.start_at) + " - " +
                    (s.end_at.has_value() ? format_ms_to_hhmm_local(*s.end_at) : "now");

                auto checkbox_item =
                    Checkbox(label, &modals_controller.edit_sessions_state.selected_ids[s.id]) | color(theme::muted);

                const auto container = Container::Vertical({
                    checkbox_item,
                });

                sessions_container->Add(Renderer(container, [checkbox_item, day_key, prev_day] {
                    return vbox({
                        prev_day != day_key
                            ? vbox({PaddingV(),
                                    text(format_day_from_utc_day_key(day_key)) | color(theme::purple) | bold,
                                    Separator(theme::panel2)})
                            : emptyElement(),
                        checkbox_item->Render(),
                    });
                }));

                prev_day = day_key;
            }
        }
    };

    const auto delete_btn = Button(
                                "Delete",
                                [on_confirm = std::move(on_confirm), rebuild_list] {
                                    on_confirm();
                                    rebuild_list();
                                },
                                ButtonOption::Ascii()) |
        color(theme::red);

    const auto cancel_btn = Button("Cancel", on_cancel, ButtonOption::Ascii());

    const auto buttons_container = Container::Horizontal({
        cancel_btn,
        delete_btn,
    });

    const auto container = Container::Vertical({
        sessions_container,
        buttons_container,
    });

    return Renderer(container, [sessions_container, delete_btn, cancel_btn, &modals_controller, rebuild_list] {
        use_effect(
            effect_state,
            [rebuild_list, cancel_btn] {
                rebuild_list();
            },
            modals_controller.triggered_value);

        return hbox({
            filler(),
            vbox({Text("Delete selected sessions") | bold | center,
                  Separator(theme::accent_soft),
                  hbox({
                      PaddingH(),
                      sessions_container->Render(),
                  }) | yframe |
                      vscroll_indicator | flex,
                  Separator(),
                  hbox({
                      cancel_btn->Render(),
                      PaddingH(),
                      delete_btn->Render(),
                  }) | center}) |
                borderStyled(ROUNDED, theme::accent_soft) | size(WIDTH, EQUAL, 35) | size(HEIGHT, GREATER_THAN, 12) |
                size(HEIGHT, LESS_THAN, 20) | clear_under,
            filler(),
        });
    });
}
