//
// Created by Oleksandr Shumihin on 27/03/2026.
//

#pragma once
#include "elements/padding.h"
#include "elements/text.h"
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

inline Element Footer() {
    return vbox({
        separator() | color(theme::panel2),
        hbox({PaddingH(),
              Text("[a] Add | [d] Delete | [e] Edit | [Space] Start/Stop | Status: [1] TODO | [2] IN PROGRESS | [3] "
                   "DONE |",
                   theme::subtle)}),
    });
}
