#pragma once
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include "core/utils/time_format.h"
#include "ui/elements/padding.h"
#include "ui/elements/task_status_badge.h"
#include "ui/styles.h"


struct ListItemProps {
    std::string title;
    int64_t duration_sec;
    TaskStatus status;
    bool is_selected = false;
    bool is_active = false;
};

inline Element ListItem(ListItemProps&& props) {
    Element el =
        hbox({(props.is_selected ? separatorStyled(HEAVY) | color(theme::blue) | size(HEIGHT, EQUAL, 3) : PaddingH()) |
                  vcenter,
              vbox({
                  vbox({
                      hbox({
                          text(props.title),
                          filler(),
                          PaddingH(2),
                      }),
                      text(""),
                      hbox({TaskStatusBadgeSmall(props.status),
                            filler(),
                            hbox({props.is_active && props.duration_sec % 2 == 0 ? text("◉ ") | color(theme::red)
                                                                                 : emptyElement(),
                                  text(format_sec_to_hhmmss(props.duration_sec)) |
                                      color(props.is_active ? theme::red : theme::muted)})}),
                  }) | borderStyled(ROUNDED, props.is_selected ? theme::blue : theme::border),
              }) | flex});

    if (props.is_selected) {
        el |= focus;
    }

    return el;
}
