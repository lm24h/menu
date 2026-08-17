#pragma once
#include <iostream>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "menu_types.hpp"

typedef std::vector<std::string> str_vec_1d_t;
typedef std::vector<std::vector<std::string>> str_vec_2d_t;

static constexpr uint8_t SINGLE_COLUMN = 1;

template <std::size_t columns>
class Menu;

template <>
class Menu<SINGLE_COLUMN>;

static constexpr auto reset_ansi_nl = "\033[0m\n";
static constexpr auto reset_ansi = "\033[0m";
static constexpr auto CLEAR_RESPONSE = 0uz;

/**
 * Checks if response is in range [low_r, high_r]
 *  @throws std::invalid_argument If response is not in given range
 */
static constexpr void RANGE_CHECK(const std::size_t low_r, const std::size_t high_r, const std::size_t response) {
    if (response < low_r || response > high_r)
        throw std::invalid_argument(
            "\nEntered response ->(" + std::to_string(response) + ") outside acceptable range [" +
            std::to_string(low_r) + ", " + std::to_string(high_r) + "]"
        );
}


enum class Align {Left, Right, Center};
enum class Color {Black, White, Red, Blue, Green, Yellow, Magenta, Cyan};
enum class Style {None, Bold, Dim, Italic, Underline};

static constexpr std::string style_driver(Style style) noexcept;
static constexpr std::string color_driver(const Color c) noexcept;

static constexpr std::string style(const Style style) noexcept {
    return "\033[" + style_driver(style) +  + "m";
}

static constexpr std::string color_text(const Color color) {
    return "\033[" + color_driver(color) + "m";
}

static constexpr std::string style_driver(const Style style) noexcept {
    switch (style) {
        case Style::Bold: return "1";
        case Style::Dim: return "2";
        case Style::Italic: return "3";
        case Style::Underline: return "4";
        default: return "";
    }
}

static constexpr std::string color_driver(const Color c) noexcept {
    switch (c) {
        case Color::Black: return "30";
        case Color::Red: return "31";
        case Color::Green: return "32";
        case Color::Yellow: return "33";
        case Color::Blue: return "34";
        case Color::Magenta: return "35";
        case Color::Cyan: return "36";
        case Color::White: return "37";
    }
    return "";
}

struct col_dimensions {
    col_dimensions(const std::size_t buff, const std::size_t pad) : buffer(buff), padding(pad) {}
    std::size_t buffer;
    std::size_t padding;
};

typedef struct {
    str_vec_2d_t items_;
    enum {NonEmpty, Empty, Uninitialized} status_;
} Menu_Items_t;

struct Color_Condition_s {
    std::string equal_condition_;
    std::size_t col_index_for_condition_;
    Color color_;
};

typedef std::vector<Color_Condition_s> Color_Conditions_t;



