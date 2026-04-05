//
// Created by Oleksandr Shumihin on 25/3/26.
//
#pragma once
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

enum class PaddingDirection {
    HORIZONTAL,
    VERTICAL,
};

template<PaddingDirection d>
inline Element Padding(const int value = 1) {
    return hbox() | size(d == PaddingDirection::HORIZONTAL ? WIDTH : HEIGHT, EQUAL, value);
}

inline Element PaddingH(const int value = 1) {
    return Padding<PaddingDirection::HORIZONTAL>(value);
}

inline Element PaddingV(const int value = 1) {
    return Padding<PaddingDirection::VERTICAL>(value);
}
