#pragma once
#include <cstddef>
#include <string>
#include <vector>

typedef std::vector<std::string> str_vec_1d_t;
typedef std::vector<std::vector<std::string>> str_vec_2d_t;

template <typename T>
concept is_2d_str_vec = std::is_same_v<std::decay_t<T>, std::vector<std::vector<std::string>>>;

template <typename T>
concept is_1d_str_vec = std::is_same_v<std::decay_t<T>, std::vector<std::string>>;

template <typename T>
concept is_menu_items = is_2d_str_vec<T> || is_1d_str_vec<T> ;

template <typename T> requires is_menu_items<T>
class Menu;

template <>
class Menu<str_vec_2d_t>;

template <>
class Menu<str_vec_1d_t>;


enum class Align {LEFT, RIGHT, CENTER};


struct col_dimensions {
    col_dimensions(const std::size_t buff, const std::size_t pad) : buffer(buff), padding(pad) {}
    std::size_t buffer;
    std::size_t padding;
};


/**
 * Holds the characteristics of a seperator
 *
 * @tparam T required type is std::vector<std::string>> or std::vector<std::vector<std::string>>>.
 * Represents the two menu types
 */
struct Separator {

    /**
     * @memberof Separator.
     * @brief Determines where to print separator.
     * Prints BEFORE index
     */
    std::size_t index;

    /** @memberof Separator. @brief character to be printed for separator */
    char separator_char;
};

