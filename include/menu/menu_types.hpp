#pragma once
#include <iostream>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "menu_types.hpp"

typedef std::vector<std::string> str_vec_1d_t;
typedef std::vector<std::vector<std::string>> str_vec_2d_t;

template <typename T>
concept is_2d_str_vec = std::is_same_v<std::decay_t<T>, std::vector<std::vector<std::string>>>;

template <typename T>
concept is_1d_str_vec = std::is_same_v<std::decay_t<T>, std::vector<std::string>>;

template <typename T>
concept is_menu_items = is_2d_str_vec<T> || is_1d_str_vec<T> ;

static constexpr uint8_t SINGLE_COLUMN = 1;

template <std::size_t columns>
class Menu;

template <>
class Menu<SINGLE_COLUMN>;

static constexpr std::string reset_ansi_nl = "\033[0m\n";
static constexpr std::string reset_ansi = "\033[0m";


enum class Align {LEFT, RIGHT, CENTER};
enum class Color {BLACK, WHITE, RED, BLUE, GREEN, YELLOW, MAGENTA, CYAN};
enum class Style {NONE, BOLD, DIM, ITALIC, UNDERLINE};

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
        case Style::BOLD: return "1";
        case Style::DIM: return "2";
        case Style::ITALIC: return "3";
        case Style::UNDERLINE: return "4";
        default: return "";
    }
}

static constexpr std::string color_driver(const Color c) noexcept {
    switch (c) {
        case Color::BLACK: return "30";
        case Color::RED: return "31";
        case Color::GREEN: return "32";
        case Color::YELLOW: return "33";
        case Color::BLUE: return "34";
        case Color::MAGENTA: return "35";
        case Color::CYAN: return "36";
        case Color::WHITE: return "37";
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
    std::size_t col_index_to_color_;
    Color color_;
    std::size_t col_index_for_condition_;
    std::string equal_condition_;
    Color else_color_;
};

struct Color_Conditions_t {
    constexpr const Color_Condition_s *find_by_col_index_to_color(const std::size_t index) const noexcept {
        for (std::size_t i = 0; i < conditions_.size(); ++i ) {
            if (conditions_[i].col_index_to_color_ == index) {
                return &conditions_[i];
            }
        }
        return nullptr;
    }
    std::vector<Color_Condition_s> conditions_;
};



