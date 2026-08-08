#ifndef MENU_LIBRARY_H
#define MENU_LIBRARY_H
#include "Menu_Stack.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>


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



struct col_characteristics {
    Align alignment{Align::LEFT};
    Align header_alignment{Align::LEFT};
    bool preceding_dots{false};
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


enum class menu_error_t {
    none,
    header_count_mismatch,
    response_already_set,
    not_in_range,
    uninitialized
};


/**
 * @brief Helper class which privately inherits the Menu class.
 * Does the calculations for the dimensions of the menu for printing
 * @tparam T required type is std::vector<std::string>> or std::vector<std::vector<std::string>>>.
 * Represents the two menu types
 */
template <is_menu_items T>
class print_helper {

public:

    print_helper() : width(), num_of_cols() {}
    /**
     * @note Constructor
     * @memberof print_helper
     * @param menu pointer to Menu class
     * @brief calculates all necessary values for printing a menu
     */
    explicit print_helper(const Menu<T> * menu);


    /**
     * @param row_index used to access menu_items for a row. Assume index starts at 1 instead of 0
     * @brief Sets the padding values for each column
     */
    void set_padding(std::size_t row_index);


    /**
     * @memberof print_helper<std::vector<std::vector<std::string>>>
     * @param row_index used to access menu_items for a row. Assume index starts at 1 instead of 0
     * @returns a completed row of the menu as a string with "\n" at end
     */
    [[nodiscard]] std::string build_row(std::size_t row_index) const;

    [[nodiscard]] std::string build_row(size_t row_index, const col_characteristics *col_chrs_ptr) const;


    /**
     * @memberof print_helper<std::vector<std::string>>
     * @param items vector of strings
     * @returns length of the longest string in vector
     */
    static constexpr std::size_t max_length_in_items(const std::vector<std::string>& items) {
        auto max_length{0uz};
        for (const auto& item : items) {
            max_length = std::max(max_length, item.length());
        }
        return max_length;
    }


    /**
     * @memberof print_helper<std::vector<std::string>>
     * @param title used for comparison to the buffer size
     * @param additional added to total buffer size
     * @returns max(total buffer size , length of title)
     */
    [[nodiscard]] constexpr std::size_t get_buffer_size(const std::string& title, const std::size_t additional=0) const {
        auto total_buffer{0uz};
        for (const auto& col : col_dimensions_) {
            total_buffer += col.buffer;
        }
        total_buffer += additional;
        return std::max(total_buffer, title.length());
    }


    /**
     * @memberof print_helper<std::vector<std::string>>
     * @param number any integer
     * @returns number of digits in integer
     */
    static constexpr size_t digit_count(int number) noexcept {
        if (number == 0)
            return 1;

        std::size_t count = number < 0 ? 1 : 0; // count '-'

        while (number != 0) {
            number /= 10;
            ++count;
        }

        return count;
    }


    /**
 * @memberof print_helper
 * @param menu_size size of menu in terms of number of menu items in vector
 * @returns number of columns necessary for the size of menu. range is [1,5]
 */
    static constexpr uint8_t get_num_of_cols(const std::size_t menu_size) noexcept
    requires is_1d_str_vec<T>
    {
        if (menu_size >= 100) return 5;
        if (menu_size >= 60)  return 4;
        if (menu_size >= 30)  return 3;
        if (menu_size >= 10)  return 2;
        return 1;
    }


    /**
     * @memberof print_helper<std::vector<std::string>>
     * @param str string to be centered
     * @param width total width to be centered in
     * @return string with left and right padding so the str is centered
     */
    static constexpr std::string center(const std::string& str, const std::size_t width) {
        if (str.size() >= width)
            return str;

        const std::size_t padding = width - str.size();
        const std::size_t left = padding / 2;
        const std::size_t right = padding - left;

        return std::string(left, ' ') + str + std::string(right, ' ');
    }


    /**
     * @param data 2D vector of strings
     * @param col index of desired column
     * @param start_index refers to the first row which is extracted
     * @return desired column as a vector of strings
     */
    static std::vector<std::string> get_col(
        const std::vector<std::vector<std::string>> &data,
        const std::size_t col,
        const std::size_t start_index=0)
    {
        std::vector<std::string> result;

        for (auto i{start_index}; i < data.size(); ++i)
        {
            result.emplace_back(data.at(i).at(col));
        }

        return result;
    }




    /** @memberof print_helper. Stores the menu items */
    std::vector<std::vector<std::string>> items;

    std::vector<col_dimensions> col_dimensions_;

    /** @memberof print_helper. Total width of the menu */
    std::size_t width;

    /** @memberof print_helper. Total number of columns */
    std::size_t num_of_cols;
};


template <>
print_helper<str_vec_2d_t>::print_helper(const Menu<str_vec_2d_t> * menu);
template <>
print_helper<str_vec_1d_t>::print_helper(const Menu<str_vec_1d_t> * menu);


/**
* @overload std::string build_row(const std::size_t row_index) const
* @memberof print_helper<std::vector<std::string>>
 */
template <>
[[nodiscard]] inline std::string print_helper<std::vector<std::string>>::build_row(
    const std::size_t row_index)
const {
    std::string row_str;
    /// for each column
    for (auto i{0uz}; i < num_of_cols; ++i) {
        /// If the index is less than the columns size
        if (row_index - 1 < items.at(i).size()) {
            /// build string for row with item at [col, row - 1] + padding for column
            row_str.append(items.at(i).at(row_index-1) + std::string(col_dimensions_.at(i).padding, ' '));
        }
    }
    row_str.append("\n");
    return row_str;
}
/**
* @overload std::string build_row(const std::size_t row_index) const
* @memberof print_helper<std::vector<std::vector<std::string>>>
 */
template <>
[[nodiscard]] inline std::string print_helper<std::vector<std::vector<std::string>>>::build_row(
    const std::size_t row_index, const col_characteristics * const col_chrs_ptr)
const {
    std::string row_str;
    /// for each column
    for (auto i{0uz}; i < num_of_cols; ++i) {

        switch (col_chrs_ptr[i].alignment) {
            case Align::LEFT:
                /// build string for row with item at [row - 1, col] + padding for column
                row_str.append(items.at(row_index-1).at(i) + std::string(col_dimensions_.at(i).padding, ' '));
                break;
            case Align::RIGHT:
                /// build string for row with padding for column + item at [row - 1, col]
                row_str.append(std::string(col_dimensions_.at(i).padding, ' ') + items.at(row_index-1).at(i));
                break;
            default:;
        }

    }
    row_str.append("\n");
    return row_str;
}



/**
 * @overload void set_padding(const std::size_t row_index)
 * @memberof print_helper<std::vector<std::string>>
 */
template <>
inline void print_helper<std::vector<std::string>>::set_padding(const std::size_t row_index) {

    /// for each column
    for (auto i{0uz}; i < num_of_cols; ++i) {

        /// if not the last column
        if (i < num_of_cols - 1) {
            /// if row index is less than or equal to the size of the column
            if (row_index <= items.at(i).size()) {
                /// padding = buffer - length of menu item
                col_dimensions_.at(i).padding = col_dimensions_.at(i).buffer - items.at(i).at(row_index-1).length();
            }
            else {
                col_dimensions_.at(i).padding = 0;
            }
        }
        /// if the last column
        else if (i == num_of_cols - 1) {
            col_dimensions_.at(i).padding = 0;
        }
    }
}
/**
 * @overload void set_padding(const std::size_t row_index)
 * @memberof print_helper<std::vector<std::vector<std::string>>>
 */
template <>
inline void print_helper<std::vector<std::vector<std::string>>>::set_padding(const std::size_t row_index) {

    /// for each column
    for (auto i{0uz}; i < num_of_cols; ++i) {
        /// padding = buffer - length of menu item
        col_dimensions_.at(i).padding = col_dimensions_.at(i).buffer - items.at(row_index-1).at(i).length();
    }
}



template <>
class Menu<str_vec_1d_t> {
public:

    ~Menu() {
        menu_item_stack_->free_stack();
        delete menu_item_stack_;
        delete title_;
        delete response_;
    }

    /**
     * Initializes menu if argument is not empty
     * @brief title and response remain empty
     * @param menu_items container of strings to be the menu choices
     */
    explicit Menu(const str_vec_1d_t &menu_items) :
        menu_item_stack_(new Menu_Item_Stack<std::string>), title_(new std::string{}), response_(new std::size_t{})
    {
        this->menu_items(menu_items);
    }

    /**
     * Initializes menu if argument is not empty
     * @brief response remains empty
     * @param menu_items container of strings to be the menu choices
     * @param title value to be title
     */
    explicit Menu(const str_vec_1d_t &menu_items, const std::string& title) :
        menu_item_stack_(new Menu_Item_Stack<std::string>),title_(new std::string(title)), response_(new std::size_t{})
    {
        this->menu_items(menu_items);
    }


    /** @brief Sets title for menu */
    constexpr void title(const std::string& title) noexcept { *title_ = title; }

    /** @returns title for menu */
    [[nodiscard]] constexpr std::string title() const noexcept { return *title_; }


    /**
     * @brief sets menu items for menu and clears response
     * @return none -- argument is not empty \n
     * @return uninitialized -- argument is empty
     */
    constexpr menu_error_t menu_items(const str_vec_1d_t& menu_items) const noexcept {
        *response_ = 0;
        if (!menu_items.empty()) {

            menu_item_stack_->init(menu_items.size());

            for (auto i{0uz}; i < menu_items.size(); ++i)
                menu_item_stack_->stack(menu_items.at(i));

            return menu_error_t::none;
        }
        return menu_error_t::uninitialized;
    }

    /** @returns menu items from menu as vector */
    [[nodiscard]] constexpr str_vec_1d_t menu_items() const noexcept {

        str_vec_1d_t menu_items = {};

        for (auto i{0uz}; i < size(); ++i) {
            menu_items.emplace_back(*menu_item_stack_->at(i));
        }
        return menu_items;
    }

    /**
     * @return number of menu items
     */
    [[nodiscard]] constexpr std::size_t size() const noexcept { return menu_item_stack_->size(); }

    /**
     * @return true if menu is empty, false O.W.
     */
    [[nodiscard]] constexpr bool empty() const noexcept { return menu_item_stack_->empty(); }

    /**
     * @brief Sets user response to menu
     * @param resp response in the form of an integer corresponding to selection
     *
     * @throws std::invalid_argument If response is less than 1 or greater than size of menu
     * @throws std::runtime_error If response is already set
     */
    constexpr void response(const std::size_t resp) {

        if (*response_ != 0)
            throw std::runtime_error("\nResponse already set ->(" + std::to_string(*response_) +
                "), call reset_response() to clear");

        if (resp < 1 || resp > this->size())
            throw std::invalid_argument(
                "\nEntered response ->(" + std::to_string(resp) + ") outside acceptable range " +
                "[1, menu_size->(" + std::to_string(this->size()) + ")]"
            );

        *response_ = resp;
    }


    /**
     * @brief Sets user response to menu. do not include numbered index
     * @param resp response in the form of a string corresponding to selection
     * @note Does not check if there is multiple of the same response options. Will set index of first found only
     * @throws std::invalid_argument If argument does not match any menu choices
     * @throws std::invalid_argument If response is less than 1 or greater than size of menu
     * @throws std::runtime_error If response is already set
     */
    constexpr void response(const std::string& resp) {

        const auto search_resp_ptr = this->find_menu_item(resp);

        if (search_resp_ptr == nullptr) {
            throw std::invalid_argument("\nEntered Response ->(" + std::string(resp) + ") Not Found " );
        }

        response(search_resp_ptr - menu_item_stack_->base_addr());
    }


    /**
     * @throws std::runtime_error if response has not been set
     * @return response as integer
     */
    [[nodiscard]] constexpr std::size_t response() const {

        if (*response_ != 0)
            return *response_;

        throw std::runtime_error("\nResponse not set");
    }

    /**
     * @throws std::runtime_error if response has not been set
     * @return response as the corresponding menu item string
     */
    [[nodiscard]] constexpr std::string response_as_menu_item() const {

        if (*response_ != 0)
            return *(menu_item_stack_->base_addr() + *response_ - 1);

        throw std::runtime_error("\nResponse not set");
    }

    /**
     * Clears all menu items, all separators, title, header(if applicable), and response
     */
    constexpr void reset() noexcept {
        menu_item_stack_->clear_menu_items();
        reset_response();
        *title_ = std::string("");
    }

    /**
     * Clears response only
     */
    constexpr void reset_response() noexcept { *response_ = 0; }

    /** @overload constexpr void print() */
    constexpr void print() const {
        print_helper print_helper_(this);

        this->print_title(&print_helper_);
        std::cout << std::string(print_helper_.width, '~') << "\n";
        this->print_rows(&print_helper_);
        std::cout << std::string(print_helper_.width, '~') << "\n";
    }


private:


    Menu_Item_Stack<std::string>* menu_item_stack_;
    std::string* const title_;
    std::size_t* const response_;



    /**
     * @brief prints title row. title is centered. If title has no value function just prints a newline
     * @param helper
     */
    void print_title(const print_helper<str_vec_1d_t> * helper) const {
        // Print title if title is assigned a value
        std::cout
                << "\n"
                << print_helper<std::vector<std::string>>::center(*title_, helper->width)
                << "\n";
    }

    [[nodiscard]] constexpr const std::string * find_menu_item(const std::string &search_item) const {
        for (auto i{0uz}; i < this->size(); ++i) {
            if (*menu_item_stack_->at(i) == search_item)
                return menu_item_stack_->at(i);
        }
        return nullptr;
    }

    /**
 * @memberof Menu<std::vector<std::string>>
 * @param helper pointer to print_helper object
 * @brief prints all rows of menu
 */
    constexpr void print_rows( print_helper<str_vec_1d_t> *const helper ) const {
        if (helper->num_of_cols == 1) {
            for (uint8_t i{0}; i < static_cast<uint8_t>(this->size()); ++i) {
                std::cout << i + 1 << ") " << *menu_item_stack_->at(i) << "\n";
            }
        }
        else {
            // Print Rows
            for (auto i{1uz}; i <= helper->items[0].size(); ++i) {
                helper->set_padding(i);
                std::cout << helper->build_row(i);
            }
        }
    }
};

template <>
class Menu<str_vec_2d_t> {

public:

    // ~~~~~~~~~~~~ Constructors / Destructors ~~~~~~~~~~~~ //
    ~Menu() {

        menu_items_->free_stack();

        free(col_chrs_ptr_);
        free(header_ptr_);

        delete menu_items_;
        delete title_ptr_;
        delete response_ptr_;
    }


    /**
     * @brief header and separators remain empty
     * @param menu_items container of strings to be the menu choices
     */
    explicit Menu(const str_vec_2d_t& menu_items) :
        separators_({}),
        menu_items_(new Menu_Item_Stack<std::vector<std::string>>),
        title_ptr_(new std::string("")),
        response_ptr_(new std::size_t())
    {
        if (!menu_items.empty())
            menu_init(menu_items);
    }

    /**
     * @brief automatically assigns separator for header
     *
     * @param menu_items container of strings to be the menu choices
     * @param header vector of strings to be headers
     *
     * @throws std::invalid_argument if the number of headers does not match the number of columns of menu.
     * headers and separators will remain empty
     */
    explicit Menu(const str_vec_2d_t& menu_items, const std::vector<std::string>& header) :
        separators_({}),
        menu_items_(new Menu_Item_Stack<std::vector<std::string>>),
        title_ptr_(new std::string("")),
        response_ptr_(new std::size_t())
    {
        if (!menu_items.empty())
            menu_init(menu_items);

        try {
            this->headers(header);
        }
        catch (const std::invalid_argument& e) {
            throw std::invalid_argument(e.what());
        }

    }


    /**
     * @brief automatically assigns separator for header
     *
     * @param menu_items container of strings to be the menu choices
     * @param header vector of strings to be headers
     * @param args
     *
     * @throws std::invalid_argument if the number of headers does not match the number of columns of menu.
     * headers and separators will remain empty
     */
    template <typename... Args>
    explicit Menu(const std::string& header, Args&&... args, const str_vec_2d_t& menu_items) :
        separators_({}),
        menu_items_(new Menu_Item_Stack<std::vector<std::string>>),
        title_ptr_(new std::string("")),
        response_ptr_(new std::size_t())
    {
        if (!menu_items.empty())
            menu_init(menu_items);

        try {
            headers(header, std::forward<Args>(args)...);
        }
        catch (const std::invalid_argument& e) {
            throw std::invalid_argument(e.what());
        }

    }


    /** @brief Sets title for menu */
    constexpr void title(const std::string& title) const noexcept { *title_ptr_ = title; }

    /** @returns title for menu or empty string if title not set */
    [[nodiscard]] constexpr std::string title() const noexcept { return *title_ptr_; }


    /** @brief sets menu items for menu and clears the header and response */
    constexpr void menu_items(const str_vec_2d_t& menu_items) {
        if (menu_items_->empty() || num_of_cols() == menu_items[0].size()) {
            *response_ptr_ = 0;
            clear_header();
            for (auto i{0uz}; i < menu_items.size(); ++i)
                menu_items_->stack(menu_items.at(i));
        }
        throw std::invalid_argument("Menu must be empty or argument must have same num of columns as current menu");
    }


    constexpr void clear_header() noexcept {
        for (auto i{0uz}; i < num_of_cols(); ++i)
            header_ptr_[i] = std::string{};
    }


    /** @returns menu items from menu as vector */
    [[nodiscard]] constexpr str_vec_2d_t menu_items() const noexcept {
        str_vec_2d_t menu_items = {};
        for (auto i{0uz}; i < size(); ++i) {
            menu_items.push_back(*menu_items_->at(i));
        }
        return menu_items;
    }


    /**
     * assigns alignment to a desired column
     * @param column_index begins at 1 -> first column is 1
     * @param alignment LEFT, RIGHT, CENTER
     */
    constexpr void align(const std::size_t column_index, const Align alignment) const {
        if (column_index > num_of_cols() || column_index < 2)
            throw std::out_of_range(
                "Entered col index ->(" + std::to_string(column_index) +
                ") outside exceptable col range ->[1, " + std::to_string(num_of_cols()) + "]"
            );
        col_chrs_ptr_[column_index - 1].alignment = alignment;
    }


    /**
     * assigns alignment to a desired header
     * @param column_index begins at 0 -> first column is 0
     * @param alignment LEFT, RIGHT, CENTER
     */
    constexpr void align_header(const std::size_t column_index, const Align alignment) const {
        if (column_index > num_of_cols() - 1 || column_index < 1)
            throw std::out_of_range(
                "Entered col index ->(" + std::to_string(column_index) +
                ") outside exceptable col range ->[1, " + std::to_string(num_of_cols()) + "]"
            );
        col_chrs_ptr_[column_index].header_alignment = alignment;
    }


    /**
     * Appends a row to the menu
     * @param row data to be appended to menu
     *
     * @throws std::invalid_argument If size of row != number of columns
     */
    constexpr void emplace_back(const std::vector<std::string>& row) {

        if (row.size() != num_of_cols())
            throw std::invalid_argument(
                "\nRow size ->(" + std::to_string(row.size()) + ") " +
                "does not match established number of columns ->(" + std::to_string(menu_items_->at(0)->size()) + ")"
            );

        menu_items_->stack(row);
    }

    /**
     * Appends a row to the menu
     * @param row data to be appended to menu
     * @param rows additional rows
     *
     * @throws std::invalid_argument If size of row != number of columns
     */
    template <typename... Args>
    constexpr void emplace_back(const std::vector<std::string>& row, const Args&... rows)
    requires (std::same_as<std::remove_cvref_t<Args>, std::vector<std::string>> && ...)
    {
        emplace_back(row);
        (emplace_back(rows), ...);
    }


    /**
     * Appends rows to the menu
     * @param rows data to be appended to menu
     *
     * @throws std::invalid_argument If size of row != number of columns
     */
    constexpr void emplace_back(const std::initializer_list<std::vector<std::string>> rows)
    {
        for (const auto & row : rows)
            emplace_back(row);
    }


    /**
     * @brief sets headers for menu c
     * @param headers describes a vector of column names
     *
     * @throws std::invalid_argument if the number of headers does not match the number of columns of menu.
     * @throws std::runtime_error If menu is empty
     */
    constexpr void headers(const std::vector<std::string>& headers)
    {
        if (menu_items_->empty() || header_ptr_ == nullptr)
            throw std::runtime_error("\nMenu must have contents to add header");
        if (headers.size() != num_of_cols())
            throw std::invalid_argument("\nHeaders count mismatch");
        for (auto i{0uz}; i < num_of_cols(); ++i)
            header_ptr_[i] = headers.at(i);
        separators_.push_back({.index=1, .separator_char='-'});
    }


    /**
     * @brief sets headers for menu and automatically assigns the necessary separator
     *
     * @param header describes a column name
     * @param args more headers
     *
     * @throws std::invalid_argument if # of headers does not match # num of columns
     */
    template <typename... Args>
    constexpr void headers(const std::string& header, Args&&... args)
    {
        static constexpr std::size_t args_size = sizeof...(args) + 1;

        if (menu_items_->empty())
            throw std::runtime_error("\nMenu must have contents to add header");
        if (args_size != num_of_cols())
            throw std::invalid_argument("\nHeader count mismatch");

        static std::string * temp_ptr = header_ptr_;
        *temp_ptr = header;
        temp_ptr++;

        if constexpr (sizeof...(args) > 0)
            headers(std::forward<Args>(args)...);

        separators_.push_back({.index=1, .separator_char='-'});
    }


    /**
     * @return headers for menu
     */
    [[nodiscard]] constexpr std::string * headers() const noexcept { return header_ptr_; }


    /**
     * @return number of menu items
     */
    [[nodiscard]] constexpr std::size_t size() const noexcept { return menu_items_->size(); }


    /**
     * @return true if menu is empty, false O.W.
     */
    [[nodiscard]] constexpr bool empty() const noexcept { return menu_items_->empty(); }


    /**
     * @return number of columns
     */
    [[nodiscard]] constexpr std::size_t num_of_cols() const noexcept {
        if (menu_items_->empty())
            return 0;
        return menu_items_->base_addr()->size();
    }


    /**
     * @brief Adds separators to menu. prints across entire width of menu.
     * If a separator already exists at specified index, function returns without assigning separator
     *
     * @param sep_char Character to be printed for seperator
     * @param index Determines where to print separator. Prints BEFORE index
     * @param args more arguments for index and character
     *
     * @throws std::invalid_argument If index is larger than menu size or index is less than 1
     */
    template <typename... Args>
    constexpr void separators(const char sep_char, const std::size_t index, Args&&... args) {

        if (this->find_separator(index) != nullptr)
            return;

        if (index > menu_items_->size() || index < 1)
            throw std::invalid_argument(
                "\nEntered index ->(" + std::to_string(index) + ") outside acceptable range " +
                "[2, menu_size->(" + std::to_string(menu_items_->size()) + ")]"
            );

        separators_.push_back({.index = index, .separator_char = sep_char});

        if constexpr (sizeof...(args) > 0)
            separators(std::forward<Args>(args)...);
    }


    /**
     * @brief removes separator at desired index
     * @param index Determines location of separator.
     * @param args more arguments for index
     */
    template <typename... Args>
    constexpr void remove_separator_at(const std::size_t index, Args&&... args) noexcept
    {
        if (const auto sep = this->find_separator(index); sep != nullptr)
            separators_.erase(separators_.begin() + (sep - separators_.data()));
        if constexpr (sizeof...(args) > 0)
            remove_separators_at(std::forward<Args>(args)...);
    }


    /**
     * @brief Sets user response to menu
     * @param resp response in the form of an integer corresponding to selection
     *
     * @throws std::invalid_argument If response is less than 1 or greater than size of menu
     * @throws std::runtime_error If response is already set
     */
    constexpr void response(const std::size_t resp) {
        if (*response_ptr_ != 0)
            throw std::runtime_error("\nResponse already set, call reset_response() to clear");
        if (resp < 1 || resp > menu_items_->size())
            throw std::invalid_argument(
                "\nEntered response ->(" + std::to_string(resp) + ") outside acceptable range " +
                "[1, menu_size->(" + std::to_string(menu_items_->size()) + ")]"
            );
        *response_ptr_ = resp;
    }

    /**
     * @throws std::runtime_error if response has not been set
     * @return response as integer
     */
    [[nodiscard]] constexpr std::size_t response() const {
        if (*response_ptr_ != 0)
            return *response_ptr_;
        throw std::runtime_error("\nResponse not set");
    }


    /**
     * Clears all menu items, all separators, title, header(if applicable), and response
     */
    constexpr void reset() noexcept {
        menu_items_->clear_menu_items();
        separators_.clear();
        this->clear_header();
        *response_ptr_ = 0;
        *title_ptr_ = "";
    }


    /**
     * Clears response only
     */
    constexpr void reset_response() noexcept { *response_ptr_ = 0; }


    /**
     * @brief Removes separator corresponding to search character
     * @param sep_char Character to be printed for seperator
     * @param args more character arguments
     */
    template <typename... Args>
    constexpr void remove_separators(const char sep_char, Args&&... args) noexcept {
        for (auto i{0uz}; i < separators_.size(); ++i) {
            if (separators_.at(i).separator_char == sep_char)
                separators_.erase(separators_.begin() + i);
        }
        if constexpr (sizeof...(args) > 0)
            separators(std::forward<Args>(args)...);
    }


    /**
 * @memberof Menu
 * @brief prints entire menu include title, separators, and rows containing the menu items
 * @note separators are printed BEFORE the entered index
 */
    constexpr void print() {
        print_helper print_helper_(this);

        this->print_title(&print_helper_);
        std::cout << std::string(print_helper_.width, '~') << "\n";
        this->print_rows(&print_helper_);
        std::cout << std::string(print_helper_.width, '~') << "\n";
    }


private:

    void menu_init(const str_vec_2d_t &menu_items) noexcept {
        menu_items_->init(menu_items.size());

        for (auto i{0u}; i < menu_items.size(); ++i)
            menu_items_->stack(menu_items.at(i));

        const auto num_cols = menu_items[0].size();

        col_chrs_ptr_ = static_cast<col_characteristics*>(
            malloc(num_cols * sizeof(col_characteristics)));
        header_ptr_ = static_cast<std::string*>(
            malloc(num_cols * sizeof(std::string)));

        for (auto i{0uz}; i < num_cols; ++i) {
            new (&col_chrs_ptr_[i]) col_characteristics();
            new (&header_ptr_[i]) std::string{};
        }
    }

    /**
     * @memberof Menu
     * @param width total num of chars to print for separator
     * @param row_index separator is printed just before this row
     * @brief prints separator
     */
    void print_separator(const std::size_t width, const std::size_t row_index) {
        const Separator *const sep = this->find_separator(row_index);
        if (sep == nullptr) return;
        std::cout << std::string(width, sep->separator_char) << "\n";
    }

    /**
     * @param row_index used to identify Separator struct
     * @return pointer to separator struct for passed index, else nullptr if not found
     */
    Separator* find_separator(const std::size_t row_index) noexcept {
        for (auto& item : separators_) {
            if (item.index == row_index) return &item;
        }
        return nullptr;
    }

    /**
 * @memberof Menu<std::vector<std::vector<std::string>>>
 * @param helper pointer to print_helper object
 * @brief prints all rows of menu
 */
    constexpr void print_rows(print_helper<str_vec_2d_t> *const helper)
    {
        if (!this->headers()->empty()) this->print_header(helper);

        // Print Rows
        for (auto i{1uz}; i <= helper->items.size(); ++i) {
            this->print_separator(helper->width, i);
            helper->set_padding(i);
            std::cout << helper->build_row(i, this->col_chrs());
        }
    }

    /**
     * @brief prints title row. title is centered. If title has no value function just prints a newline
     * @param helper
     */
    void print_title(const print_helper<str_vec_2d_t> * helper) {
        // Print title if title is assigned a value
        std::cout
                << "\n"
                << print_helper<std::vector<std::string>>::center(*title_ptr_, helper->width)
                << "\n";
    }


    /** @brief Prints the header row. Does not check if header has elements */
    constexpr void print_header(const print_helper<str_vec_2d_t> *const helper) {
        std::string row_str;
        /// for each column
        for (auto i{0uz}; i < helper->num_of_cols; ++i) {

            switch ((col_chrs_ptr_+ i)->header_alignment) {
                case Align::LEFT:
                    /// build string for row with item length at [row - 1, col] + padding for column
                    row_str.append(
                        header_ptr_[i] + std::string(helper->col_dimensions_.at(i).buffer - header_ptr_[i].length(), ' '));
                    break;
                case Align::RIGHT:
                    // build string for row with padding for column + item length at [row - 1, col]
                    row_str.append(
                        std::string(helper->col_dimensions_.at(i).buffer - header_ptr_[i].length(), ' ') + header_ptr_[i]);
                    break;
                default:;
            }

        }
        row_str.append("\n");
        std::cout << row_str;
    }


    [[nodiscard]] constexpr col_characteristics* col_chrs() const {return col_chrs_ptr_;}


    std::vector<Separator> separators_;
    Menu_Item_Stack<std::vector<std::string>>* menu_items_;
    std::string* header_ptr_;
    col_characteristics* col_chrs_ptr_;
    std::string* const title_ptr_;
    std::size_t* const response_ptr_;

};





template <>
inline print_helper<str_vec_1d_t>::print_helper(const Menu<str_vec_1d_t> * const menu)  {

    auto menu_items = menu->menu_items();
    auto slice{0uz};
    uint8_t remainder{0};
    const auto size = menu_items.size();
    std::vector<std::size_t> it_positions;


    // adding numbered index to all menu items, format: #) menu_item
    for (auto i{0uz}; i < menu_items.size(); ++i) {
        menu_items.at(i).insert(0, std::to_string(i + 1) + ") ");
    }

    num_of_cols = get_num_of_cols(size);

    // slice is used to determine the interval of indexes to make into individual columns
    slice = size / num_of_cols;

    /*
     *  remainder determines if a column needs an extra spot assuming the number of
     *  elements in menu is not dividable by the number of columns
     */
    remainder = size % num_of_cols;


    // Calculate the positions for the slicing for the columns
    for (auto i{0uz}; i < num_of_cols; ++i) {
        if (i==0)
            it_positions.emplace_back(slice + (remainder > i));
        else
            it_positions.emplace_back(it_positions.at(i - 1) + slice + (remainder > i));
    }


    if (num_of_cols > 1) {

        // create the first column and calculate buffer
        items.emplace_back(menu_items.begin(), menu_items.begin() + it_positions[0]);
        col_dimensions_.emplace_back(max_length_in_items(items[0]) + 8, 0);

        /*
         * This loop only applies if num_of_cols >= 3.
         * creates all columns that are between the first and the last and calculates buffers
         */
        for (uint8_t i = 0; i < num_of_cols - 2; ++i) {
            items.emplace_back(menu_items.begin() + it_positions[i], menu_items.begin() + it_positions[i+1]);
            col_dimensions_.emplace_back(max_length_in_items(items[i + 1]) + 8, 0);
        }

        // create the last column. buffer not necessary
        items.emplace_back(menu_items.begin() + it_positions[num_of_cols - 2], menu_items.end());
        col_dimensions_.emplace_back(max_length_in_items(items[num_of_cols - 1]), 0);

        width = this->get_buffer_size(menu->title());
    }
    else
        width = this->get_buffer_size(menu->title(), max_length_in_items(menu_items));
}
template <>
inline print_helper<str_vec_2d_t>::print_helper(const Menu<str_vec_2d_t> * const menu)  {

    items = menu->menu_items();
    num_of_cols = items.at(0).size();

    // adding numbered index to menu items in first column, format: #) menu_item[0]
    for (auto i{0uz}; i < items.size(); ++i) {
        items.at(i).at(0).insert(0, std::to_string(i + 1) + ") ");
    }

    for (auto i{0uz}; i < num_of_cols; ++i) {

        auto test_column = get_col(items, i);

        // if there are headers then they need to be included in buffer calculation
        if (const auto h = menu->headers(); !h->empty())
            test_column.emplace_back(*(h + i));

        col_dimensions_.emplace_back(max_length_in_items(test_column) + 8, 0);
    }

    width = this->get_buffer_size(menu->title());
}


#endif // MENU_LIBRARY_H