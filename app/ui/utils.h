//
// Created by Oleksandr Shumihin on 31/03/2026.
//
#pragma once
#include <ftxui/component/mouse.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>


template <typename T = int64_t> struct EffectState {
    T prev_value{};
    bool initialized = false;
};

template <typename T> void use_effect(EffectState<T>& state, std::function<void()>&& cb, const T& arg) {
    if (!state.initialized || state.prev_value != arg) {
        state.initialized = true;
        state.prev_value = arg;
        cb();
    }
}

using ftxui::Box;
using ftxui::Mouse;

inline bool is_mouse_inside_box(const Mouse& mouse, const Box& box) {
    return mouse.x >= box.x_min && mouse.x <= box.x_max && mouse.y >= box.y_min && mouse.y <= box.y_max;
}

struct ScrollState {
    Box scroll_box;
    float offset_y = 0.0f;

    bool handle_mouse(const Mouse& mouse) {
        if (!is_mouse_inside_box(mouse, scroll_box)) {
            return false;
        }

        if (mouse.button == Mouse::WheelDown) {
            offset_y = std::min(1.0f, offset_y + 0.05f);
            return true;
        }

        if (mouse.button == Mouse::WheelUp) {
            offset_y = std::max(0.0f, offset_y - 0.05f);
            return true;
        }
        return false;
    }

    void reset_offset() {
        offset_y = 0.0f;
    }
};
