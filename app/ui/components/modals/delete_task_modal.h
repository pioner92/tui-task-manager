#pragma once
#include "ui/elements/separator.h"
#include "ui/elements/text.h"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "core/controllers/modals_controller.h"
#include "ui/utils.h"

using namespace ftxui;

inline Component DeleteTaskModal(const ModalsController& modals_controller,
                                 std::function<void()>&& on_confirm,
                                 std::function<void()>&& on_cancel) {
    const auto cancel_btn = Button("Cancel", on_cancel, ButtonOption::Ascii());
    const auto delete_btn = Button("Delete", on_confirm, ButtonOption::Ascii()) | color(theme::red);

    static EffectState effect_state;

    const auto container = Container::Horizontal({
        cancel_btn,
        delete_btn,
    });

    return Renderer(container, [container, delete_btn, cancel_btn, &modals_controller] {
        // Focus on cancel button
        use_effect(
            effect_state,
            [container, cancel_btn] {
                container->SetActiveChild(cancel_btn);
            },
            modals_controller.triggered_value);

        return hbox({filler(),
                     vbox({
                         Text("Delete Task ?") | bold | center,
                         Separator(theme::accent_soft),
                         filler(),
                         hbox({
                             cancel_btn->Render(),
                             text(" "),
                             delete_btn->Render(),
                         }) | center,
                     }) | borderStyled(BorderStyle::ROUNDED, theme::accent_soft) |
                         size(WIDTH, EQUAL, 35) | size(HEIGHT, EQUAL, 6) | clear_under,
                     filler()});
    });
}
