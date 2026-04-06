//
// Created by Oleksandr Shumihin on 31/03/2026.
//

#pragma once
#include "ui/elements/padding.h"
#include "ui/elements/separator.h"
#include "ui/styles.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

inline Element Description(const std::optional<std::string>& description, const Component& copy_btn) {
    if (!description || description.value().empty()) {
        return emptyElement();
    }

    return vbox({hbox({
                     text("Notes ") | color(theme::purple) | bold,
                     copy_btn->Render(),
                 }),
                 vbox({
                     PaddingV(),
                     hbox({paragraph(description.value()) | color(theme::text)}),
                     PaddingV(),
                 }),
                 Separator(theme::panel2)});
}
