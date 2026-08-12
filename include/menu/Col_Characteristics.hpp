#pragma once
#include "menu_types.hpp"

struct Col_Characteristics {
    Col_Characteristics()=default;
    explicit Col_Characteristics(
        const Align alignment,
        const Style style,
        const Color color) noexcept :

        alignment_(alignment),
        style_(style),
        color_(color) {}

    Align alignment_{Align::LEFT};
    Style style_{Style::NONE};
    Color color_{Color::WHITE};
};