#pragma once
#include "menu_types.hpp"
#include "forward_list"

/**
 * Holds the characteristics of a seperator, including: \n
 * Index \n Char \n Color
 */

struct Sep_Characteristics {
    Sep_Characteristics() : index_{0uz}, separator_char_{' '}, color_{Color::White} {}
    Sep_Characteristics(const std::size_t index, const char sep_char, const Color color) :
        index_{index}, separator_char_{sep_char}, color_{color} {}
    /**
     * @memberof Separator.
     * @brief Determines where to print separator.
     * Prints BEFORE index
     */
    std::size_t index_;

    /** @memberof Separator. @brief character to be printed for separator */
    char separator_char_;

    /** @memberof Separator. @brief color of separator */
    Color color_;
};


/**
 * Contains a forward list with each node containing information for a separator @see Sep_Characteristics
 */
struct Separator {

    Separator() : separators_{Sep_Characteristics{}} {}


    /**
     * @param row_index used to identify Sep_Characteristics node
     * @return pointer to Sep_Characteristics node for passed index
     * @return nullptr if not found
     */
    [[nodiscard]]
    constexpr const Sep_Characteristics* find_separator(const std::size_t row_index) const noexcept {
        for (auto& sep : separators_) {
            if (sep.index_ == row_index) return &sep;
        }
        return nullptr;
    }


    /**
     * @brief removes separator at desired index
     * @param index Determines location of separator.
     * @param args more arguments for index
     */
    template <typename... Args>
    constexpr void remove_separator_at(const std::size_t index, Args&&... args) noexcept {
        separators_.remove_if([index](const Sep_Characteristics& sep) {
            return sep.index_ == index;
        });
        if constexpr (sizeof...(args) > 0)
            remove_separators_at(std::forward<Args>(args)...);
    }


    /**
     *
     * @param sep_char Character to be printed for seperator
     * @param color prints char in this color
     * @param index location of separator
     */
    constexpr void add(const char sep_char, const Color color, const std::size_t index) noexcept {
        separators_.emplace_front(index, sep_char, color);
    }


    /**
     * @brief Removes every separator corresponding to search character
     * @param sep_char Character to be printed for seperator
     */
    constexpr void remove_separator(const char sep_char) noexcept {
        separators_.remove_if([sep_char](const Sep_Characteristics& sep) {
            return sep.separator_char_ == sep_char;
        });
    }

    /**
     * @param width total num of chars to print for separator
     * @param row_index separator is printed just before this row
     * @brief prints separator
     */
    constexpr void print(const std::size_t width, const std::size_t row_index) const {
        const auto sep = find_separator(row_index);
        if (sep == nullptr)
            return;

        const auto color = color_text(sep->color_);
        std::string output;
        output.reserve(color.size() + width + std::strlen(reset_ansi_nl));

        output += color;
        output.append(width, sep->separator_char_);
        output += reset_ansi_nl;

        std::fwrite(output.data(), 1, output.size(), stdout);
    }

    /** Clears all separators */
    constexpr void clear() noexcept { separators_.clear(); }

    constexpr std::size_t count() noexcept {
        return std::distance(separators_.begin(), separators_.end()) - 1;
    }


    /** @memberof Separator \n nodes in list contain info on a separator for menu */
    std::forward_list<Sep_Characteristics> separators_;
};