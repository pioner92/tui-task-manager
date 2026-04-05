//
// Created by Oleksandr Shumihin on 24/3/26.
//

#pragma once
#include "core/controllers/modals_controller.h"
#include "ui/elements/separator.h"
#include "ui/elements/text.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "ui/components/custom_input.h"
#include "ui/styles.h"

using namespace ftxui;

inline Component CreateTaskModal(CreateTaskModalState& modal_state,
                              std::function<void(std::string& title, std::string& description)>&& on_save,
                              std::function<void()>&& on_cancel) {
    auto input_title = CustomInput(&modal_state.title, "Title");
    auto input_description = CustomInput(&modal_state.description, "Description");

    const auto save_btn = Button(
        "Create",
        [on_save = std::move(on_save), &modal_state] {
            on_save(modal_state.title, modal_state.description);
        },
        ButtonOption::Ascii());

    const auto cancel_btn = Button("Cancel", on_cancel, ButtonOption::Ascii());

    const auto container = Container::Vertical({input_title,
                                                input_description,
                                                Container::Horizontal({
                                                    cancel_btn,
                                                    save_btn,
                                                })});


    return Renderer(container, [input_title, input_description, save_btn, cancel_btn]() {
        return hbox({filler(),
                     vbox({
                         Text("Create Task") | bold | center,
                         input_title->Render() | bold,
                         input_description->Render() | bold,
                         Separator(),
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
