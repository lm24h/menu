#pragma once
#include <array>
#include <stdexcept>
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


    Align alignment_{Align::Left};
    Style style_{Style::None};
    Color color_{Color::White};
};

struct Header_t {
    Header_t()=default;
    std::string header_str;
    Col_Characteristics characteristics;
};

struct Column_t {
    Column_t()=default;
    Header_t header;
    Color_Condition_s color_conditions;
    Col_Characteristics characteristics;
    std::size_t buffer{0uz};
    bool preceding_dots{false};
};


/**
 * This Class holds all the general styling and sizing information for each column
 * @tparam size number of columns of menu
 */
template <std::size_t size>
class Columns {
public:

    Columns() {
        clear_headers();
    }

    constexpr void style_headers(
        const std::size_t col_index,
        const Align alignment=Align::Left,
        const Color color=Color::White,
        const Style style=Style::None)
    {
        columns_[col_index].header.characteristics.alignment_ = alignment;
        columns_[col_index].header.characteristics.style_ = style;
        columns_[col_index].header.characteristics.color_ = color;
    }

    constexpr void set_headers(std::ranges::input_range auto&& headers) {
        if (std::ranges::size(headers) != size)
            throw std::invalid_argument("\nNumber of header arguments ->(" +
                std::to_string(std::ranges::size(headers)) +
                ") > number of cols ->(" + std::to_string(size) + ")");
        for (std::size_t i = 0; i < size; ++i)
            columns_[i].header.header_str = headers[i];
    }

    constexpr void set_headers(const std::initializer_list<std::string>& headers) {
        if (headers.size() > size)
            throw std::out_of_range(
                "\nNumber of header arguments ->(" + std::to_string(headers.size()) +
                ") > number of cols ->(" + std::to_string(size) + ")");
        auto i{0uz};
        for (const auto& header : headers)
            columns_[i++].header.header_str = header;
    }

    std::string get_header(const std::size_t index) {
        if (index > size || index == 0)
            throw std::out_of_range(
                "Entered col index ->(" + std::to_string(index) +
                ") outside exceptable col range ->[1, " + std::to_string(size) + "]"
            );
        return columns_[index - 1].header.header_str;
    }


    [[nodiscard]]
    Align get_header_alignment(const std::size_t index) {
        if (index > size || index == 0)
            throw std::out_of_range(
                "Entered col index ->(" + std::to_string(index) +
                ") outside exceptable col range ->[1, " + std::to_string(size) + "]"
            );
        return columns_[index - 1].header.characteristics.alignment_;
    }


    [[nodiscard]]
    std::array<Column_t, size> columns() noexcept {
        return columns_;
    }


    constexpr bool header_is_empty() noexcept {
        for (const auto& col : columns_)
            if (col.header.header_str != "")
                return false;
        return true;
    }

    constexpr void clear_headers() noexcept {
        for (auto& col : columns_)
            col.header.header_str = "";
    }


    /**
     * assigns alignment to a desired header
     * @param col_index begins at 1 -> first column is 1
     * @param alignment LEFT, RIGHT, CENTER
     * @param args
     */
    template <typename... Args>
    constexpr void align_header(const std::size_t col_index, const Align alignment, Args&&... args) {
        if (col_index > size || col_index < 1)
            throw std::out_of_range(
                "Entered col index ->(" + std::to_string(col_index) +
                ") outside exceptable col range ->[1, " + std::to_string(size) + "]"
            );
        columns_[col_index - 1].header.characteristics.alignment_ = alignment;
        if constexpr (sizeof...(args))
            align_header(std::forward<Args>(args)...);
    }

    /**
     * assigns alignment to a desired column
     * @param col_index begins at 1 -> first column is 1begins at 1 -> first column is 1
     * @param alignment LEFT, RIGHT, CENTER
     * @param args more col_index and alignment arguments
     */
    template <typename... Args>
    constexpr void align_column(
        const std::size_t col_index,
        const Align alignment,
        Args&&... args)
    {
        if (col_index > size || col_index < 1) {
            for (auto i{0uz}; i < size; ++i)
                columns_[i].characteristics.alignment_ = Align::Left;
            throw std::out_of_range(
                "Entered col index ->(" + std::to_string(col_index) +
                ") outside exceptable col range ->[1, " + std::to_string(size) + "]"
            );
        }

        columns_[col_index - 1].characteristics.alignment_ = alignment;

        if constexpr (sizeof...(args)) {
            align_column(std::forward<Args>(args)...);
        }
    }



    /**
     * Adds preceding dots to desired column
     * @param column begins at 1 <-> first column is 1
     * @throws std::out_of_range if entered column is not valid
     */
    constexpr void preceding_dots( const std::size_t column ) {
        if (column < 1 || column > size)
            throw std::out_of_range(
                "\nEntered column index -> (" + std::to_string(column) +
                ") not in acceptable range -> [1, " + std::to_string(size) + "]");
        columns_[column - 1].preceding_dots = true;
    }


    /** @brief Prints the header row. Does not check if header has elements */
    constexpr void print_header(const std::vector<col_dimensions> &dimensions) {

        std::string row_str;
        const auto pad = new std::size_t{0uz};

        for (auto i{0uz}; i < size; ++i) {

            *pad = dimensions.at(i).buffer - get_header(i + 1).length();
            row_str.append(
                color_text(columns_[i].header.characteristics.color_) + style(columns_[i].header.characteristics.style_));

            switch (columns_[i].header.characteristics.alignment_) {
                case Align::Left:
                    row_str.append(get_header(i + 1) + std::string(*pad, ' '));
                    break;
                case Align::Right:
                    row_str.append(std::string(*pad, ' ') + get_header(i + 1));
                    break;
                default: // Center
                    const auto half_pad = *pad / 2;
                    const auto remainder = *pad % 2;
                    row_str.append(
                        std::string(half_pad, ' ') +
                        get_header(i + 1) +
                        std::string(half_pad + remainder, ' '));
                    break;
            }
        }
        delete pad;
        row_str.append(reset_ansi_nl);
        std::cout << row_str;
    }


    constexpr void set_color_conditions(
        const std::size_t col_index_to_color,
        const Color color,
        const std::size_t col_index_for_condition,
        const std::string& equal_condition,
        const Color else_color=Color::White)
    {
        if (col_index_to_color > size || col_index_to_color < 1)
            throw std::invalid_argument("Entered col index ->(" + std::to_string(col_index_to_color) +
                ") outside exceptable col range ->[1, " + std::to_string(size) + "]");
        if (col_index_for_condition > size || col_index_for_condition < 1)
            throw std::invalid_argument("Entered col index ->(" + std::to_string(col_index_for_condition) +
                ") outside exceptable col range ->[1, " + std::to_string(size) + "]");

        columns_[col_index_to_color - 1].color_conditions = {
            .equal_condition_ = equal_condition,
            .col_index_for_condition_ = col_index_for_condition,
            .color_ = color};
        auto dummy = 0;
    }

    constexpr const Color_Condition_s* get_color_condition(const std::size_t col_index) const {
        return &columns_[col_index - 1].color_conditions;
    }


private:
    std::array<Column_t, size> columns_;
};