//
// Created by Oleksandr Shumihin on 25/3/26.
//

#pragma once
#include "ui/styles.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

inline ButtonOption button_style() {
    auto option = ButtonOption::Ascii();
    option.transform = [](const EntryState& s) {
        auto element = text(s.label);
        if (s.focused) {
            element |= color(theme::red);
        } else {
            element |= color(theme::muted);
        }
        return element | center | borderEmpty | flex;
    };
    return option;
}

inline Component CustomInput(std::string* value, const char* placeholder) {
    InputOption options;

    options.multiline = true;
    auto clear_button = Button(
        "x",
        [value] {
            value->clear();
        },
        button_style());

    options.transform = [](InputState s) {
        s.element = s.element | color(s.is_placeholder ? theme::muted : theme::text);

        if (s.focused) {
            s.element |= borderStyled(ROUNDED, theme::blue);
        } else {
            s.element = s.element | borderStyled(ROUNDED, theme::subtle);
        }


        return s.element;
    };

    auto input = Input(value, placeholder, options);

    const auto container = Container::Stacked({
        input,
        clear_button,
    });

    return Renderer(container, [input, clear_button] {
        return hbox({
            input->Render(),
            clear_button->Render() | vcenter,
        });
    });
}
