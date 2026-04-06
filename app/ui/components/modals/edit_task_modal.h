//
// Created by Oleksandr Shumihin on 24/3/26.
//

#pragma once
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "core/controllers/modals_controller.h"
#include "ui/components/custom_input.h"
#include "ui/elements/separator.h"
#include "ui/elements/text.h"
#include "ui/styles.h"
#include "ui/utils.h"

using namespace ftxui;

inline Component EditTaskModal(const ModalsController& modal_controller,
                               std::function<void(std::string& title, std::string& description)>&& on_save,
                               std::function<void()>&& on_cancel) {
    auto input_title = CustomInput(&modal_controller.modal_state.title, "Title");
    auto input_description = CustomInput(&modal_controller.modal_state.description, "Description");
    static EffectState effect_state;

    const auto save_btn = Button(
        "Save",
        [on_save = std::move(on_save), &modal_controller] {
            on_save(modal_controller.modal_state.title, modal_controller.modal_state.description);
        },
        ButtonOption::Ascii());

    const auto cancel_btn = Button("Cancel", on_cancel, ButtonOption::Ascii());

    const auto container = Container::Vertical({input_title,
                                                input_description,
                                                Container::Horizontal({
                                                    cancel_btn,
                                                    save_btn,
                                                })});

    return Renderer(container, [container, input_title, input_description, save_btn, cancel_btn, &modal_controller]() {
        use_effect(
            effect_state,
            [&container, &input_title] {
                container->SetActiveChild(input_title);
            },
            modal_controller.triggered_value);

        return hbox({filler(),
                     vbox({
                         Text("Edit Task") | bold | center,
                         input_title->Render() | bold,
                         input_description->Render() | bold,
                         Separator(theme::accent_soft),
                         hbox({
                             cancel_btn->Render() | color(theme::muted),
                             text(" "),
                             save_btn->Render() | color(theme::green),
                         }) | center,
                     }) | borderStyled(BorderStyle::ROUNDED, theme::accent_soft) |
                         size(WIDTH, EQUAL, 60) | clear_under,
                     filler()});
    });
}
