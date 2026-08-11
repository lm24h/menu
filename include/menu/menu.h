#ifndef MENU_LIBRARY_H
#define MENU_LIBRARY_H
#include "menu_types.hpp"
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>

enum class menu_error_t {
    none,
    header_count_mismatch,
    response_already_set,
    not_in_range,
    uninitialized
};

struct Title_t {
    ~Title_t() {
        std::free(title_);
    }
    Title_t(const Title_t& other) {
        length_ = other.length_;
        title_ = static_cast<char*>(std::malloc(length_ + 1));
        for (auto i{0uz}; i < length_; ++i)
            title_[i] = other.title_[i];
        title_[length_] = '\0';
    }
    Title_t(Title_t&& other) noexcept : title_{other.title_}, length_{other.length_} {
        other.length_ = 0;
        other.title_ = nullptr;
    }
    explicit Title_t(const std::string& title) :
    title_{static_cast<char*>(std::malloc(title.length() + 1))},
    length_{static_cast<unsigned int>(title.length())}
    {
        for (auto i{0uz}; i < length_; ++i)
            title_[i] = title[i];
        title_[length_] = '\0';
    }

    Title_t& operator=(const Title_t& other) {
        if (this == &other)
            return *this;

        char* new_title = static_cast<char*>(std::malloc(other.length_ + 1));

        for (auto i{0uz}; i < other.length_; ++i)
            new_title[i] = other.title_[i];
        new_title[other.length_] = '\0';

        std::free(title_);
        title_ = new_title;
        length_ = other.length_;

        return *this;

    }
    Title_t& operator=(Title_t&& other) noexcept {
        if (this == &other)
            return *this;

        std::free(title_);
        title_ = other.title_;
        length_ = other.length_;
        other.title_ = nullptr;
        other.length_ = 0;

        return *this;
    }

    constexpr std::string str() const noexcept {
        std::string return_str{""};
        for (auto i{0u}; i < length_; ++i)
            return_str += title_[i];
        return return_str;
    }

    constexpr void set_title(const std::string& title) noexcept {
        title_ = static_cast<char*>(std::realloc(title_, sizeof(char) * title.length()));
        length_ = static_cast<unsigned int>(title.length());
        for (auto i{0uz}; i < length_; ++i) {
            new (title_ + i) char(title[i]);
        }
    }

    constexpr void set_align(const Align alignment) noexcept { title_alignment_ = alignment; }
    constexpr Align get_align() const noexcept { return title_alignment_; }
private:
    char* title_;
    unsigned int length_;
    Align title_alignment_{Align::CENTER};
};

namespace Menu_Characteristics {
    struct column {
        Align alignment{Align::LEFT};
        Align header_alignment{Align::LEFT};
        bool preceding_dots{false};
    };
    struct row {
        inline static std::initializer_list<std::size_t> exclude_from_dots;
        inline static std::initializer_list<std::size_t> exclude_from_alignment;
    };
}

static constexpr uint8_t FIRST_COLUMN = 0;


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

    [[nodiscard]] std::string build_row(size_t row_index, const Menu_Characteristics::column *col_chrs_ptr) const;


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
        const std::size_t half = padding / 2;
        const std::size_t rem = padding % 2;

        return std::string(half, ' ') + str + std::string(half + rem, ' ');
    }


    /**
     * @memberof print_helper<std::vector<std::string>>
     * @param str string to be right aligned
     * @param width total width to be aligned in
     * @return string with left padding so the str is right aligned
     */
    static constexpr std::string right(const std::string& str, const std::size_t width) {
        if (str.size() >= width)
            return str;
        const std::size_t padding = width - str.size();
        return std::string(padding, ' ') + str;
    }


    /**
     * @memberof print_helper<std::vector<std::string>>
     * @param str string to be right aligned
     * @param width total width to be aligned in
     * @return string with left padding so the str is right aligned
     */
    static constexpr std::string left(const std::string& str, const std::size_t width) {
        if (str.size() >= width)
            return str;
        const std::size_t padding = width - str.size();
        return str + std::string(padding, ' ');
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


    bool in_list(const std::size_t row, const std::initializer_list<std::size_t>& excl_rows) const {
        for (const auto& excl_row : excl_rows)
            if (excl_row == row)
                return true;
        return false;
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
    const std::size_t row_index, const Menu_Characteristics::column * const col_chrs_ptr) const {

    std::string row_str;
    std::string item{};
    auto * const pad = new std::size_t{0uz};
    char preceding_pad_char, follow_pad_char;

    for (auto i{0uz}; i < num_of_cols; ++i) {

        *pad = col_dimensions_.at(i).buffer - items[row_index - 1][i].length();

        const Align alignment = in_list(row_index, Menu_Characteristics::row::exclude_from_alignment)
                              ? Align::LEFT : col_chrs_ptr[i].alignment;

        // Index must be separated from the item string for first column
        if (i == FIRST_COLUMN) {
            const auto index_pos = new std::size_t{items[row_index - 1][i].find(')')};
            item = items[row_index - 1][i].substr(*index_pos + 2);
            row_str.append(items[row_index - 1][i].substr(0, *index_pos + 2));
            delete index_pos;
        }
        else {
            item = items[row_index - 1][i];
        }

        switch (alignment) {
            case Align::LEFT:
                if (
                    i != num_of_cols - 1 &&
                    col_chrs_ptr[i + 1].preceding_dots &&
                    !in_list(row_index, Menu_Characteristics::row::exclude_from_dots)
                )
                    preceding_pad_char = '.';
                else
                    preceding_pad_char = ' ';
                row_str.append(item + std::string(*pad, preceding_pad_char));
                break;
            case Align::RIGHT:
                if (col_chrs_ptr[i].preceding_dots && !in_list(row_index, Menu_Characteristics::row::exclude_from_dots))
                    follow_pad_char = '.';
                else
                    follow_pad_char = ' ';
                row_str.append(std::string(*pad, follow_pad_char) + item);
                break;
            case Align::CENTER:

                const auto half_pad = *pad / 2;
                const auto remainder = *pad % 2;

                if (
                    i != num_of_cols - 1 &&
                    col_chrs_ptr[i + 1].preceding_dots &&
                    !in_list(row_index, Menu_Characteristics::row::exclude_from_dots)
                )
                    follow_pad_char = '.';
                else
                    follow_pad_char = ' ';
                if (col_chrs_ptr[i].preceding_dots)
                    preceding_pad_char = '.';
                else
                    preceding_pad_char = ' ';

                row_str.append(
                    std::string(half_pad, preceding_pad_char) +
                    item +
                    std::string(half_pad + remainder, follow_pad_char));
                break;
        }
    }
    delete pad;
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
class Menu<str_vec_1d_t> {
public:

    ~Menu() {
        delete menu_items_;
        delete title_;
        delete response_;
    }

    /**
     * Initializes menu if argument is not empty
     * @brief title and response remain empty
     * @param menu_items container of strings to be the menu choices
     */
    explicit Menu(const str_vec_1d_t &menu_items) :
        menu_items_(new str_vec_1d_t{menu_items}), title_(new std::string{}), response_(new std::size_t{0})
    {}

    /**
     * Initializes menu if argument is not empty
     * @brief response remains empty
     * @param menu_items container of strings to be the menu choices
     * @param title value to be title
     */
    explicit Menu(const str_vec_1d_t &menu_items, const std::string& title) :
        menu_items_(new str_vec_1d_t{menu_items}),title_(new std::string(title)), response_(new std::size_t{0})
    {}

    /** @brief Sets title for menu */
    constexpr void title(const std::string& title) noexcept { *title_ = title; }

    /** @returns title for menu */
    [[nodiscard]] constexpr std::string title() const noexcept { return *title_; }


    /**
     * @brief sets menu items for menu and clears response
     * @return none -- argument is not empty \n
     * @return uninitialized -- argument is empty
     */
    constexpr void menu_items(const str_vec_1d_t& menu_items) const noexcept {
        *response_ = 0;
        *menu_items_ = menu_items;
    }

    /** @returns menu items from menu as vector */
    [[nodiscard]] constexpr str_vec_1d_t menu_items() const noexcept { return *menu_items_; }

    /**
     * @return number of menu items
     */
    [[nodiscard]] constexpr std::size_t size() const noexcept { return menu_items_->size(); }

    /**
     * @return true if menu is empty, false O.W.
     */
    [[nodiscard]] constexpr bool empty() const noexcept { return menu_items_->empty(); }

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

        if (resp < 1 || resp > menu_items_->size())
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
        const auto search_resp_ptr = find_menu_item(resp);
        if (search_resp_ptr == nullptr) {
            throw std::invalid_argument("\nEntered Response ->(" + std::string(resp) + ") Not Found " );
        }
        response(search_resp_ptr - &menu_items_->front());
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
            return menu_items_->at(*response_ - 1);
        throw std::runtime_error("\nResponse not set");
    }

    /**
     * Clears all menu items, all separators, title, header(if applicable), and response
     */
    constexpr void reset() noexcept {
        menu_items_->clear();
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

    str_vec_1d_t* const menu_items_;
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
            if (menu_items_->at(i) == search_item)
                return &menu_items_->at(i);
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
                std::cout << i + 1 << ") " << menu_items_->at(i) << "\n";
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


/**
 * Represents a 2D Menu consisting of rows with a set number of columns. \n
 * Headers for columns and menu title are optional. \n
 * Columns, headers, and title can be aligned, among other customization options. \n
 * Response to menu must be set before it is retrieved . \n
 * After object is constructed, the number of columns can not be changed unless entire object is reset.
 */
template <>
class Menu<str_vec_2d_t> {

public:
    /**
     * Destructor -- Frees up all previously allocated memory
     */
    ~Menu() {
        free(col_chrs_ptr_);
        delete menu_items_;
        delete response_ptr_;
        delete separators_;
        delete header_ptr_;
        if (title_ptr_ != nullptr) {
            delete title_ptr_;
        }
    }


    /**
     * Copy Constructor -- Creates new copy of a Menu object
     * @param other any other Menu object
     */
    Menu(const Menu& other) :
        menu_items_{new str_vec_2d_t{*other.menu_items_}},
        separators_{new std::vector{*other.separators_}},
        header_ptr_{new std::vector{*other.header_ptr_}},
        col_chrs_ptr_{nullptr},
        title_ptr_{nullptr},
        response_ptr_{new std::size_t{*other.response_ptr_}}
    {
        if (other.title_ptr_ != nullptr)
            title_ptr_ = new Title_t{*other.title_ptr_};

        col_chrs_ptr_ = static_cast<Menu_Characteristics::column*>(
            malloc(num_of_cols() * sizeof(Menu_Characteristics::column)));

        for (auto i{0uz}; i < num_of_cols(); ++i) {
            new (&col_chrs_ptr_[i]) Menu_Characteristics::column{other.col_chrs_ptr_[i]};
        }
    }


    /**
     * Move Constructor -- Transfers ownership of memory and nulls other object
     * @param other any other Menu object
     */
    Menu(Menu&& other) noexcept :
        menu_items_{other.menu_items_},
        separators_{other.separators_},
        header_ptr_{other.header_ptr_},
        col_chrs_ptr_{other.col_chrs_ptr_},
        title_ptr_{other.title_ptr_},
        response_ptr_{other.response_ptr_}
    {
        other.menu_items_ = nullptr;
        other.separators_ = nullptr;
        other.header_ptr_ = nullptr;
        other.col_chrs_ptr_ = nullptr;
        other.title_ptr_ = nullptr;
        other.response_ptr_ = nullptr;
    }



    /**
     * @brief header and separators remain empty
     * @param menu_items container of strings to be the menu choices
     */
    explicit Menu(const str_vec_2d_t& menu_items) :
        menu_items_(new str_vec_2d_t{menu_items}),
        separators_(new std::vector<Separator>),
        header_ptr_(new std::vector<std::string>),
        col_chrs_ptr_(nullptr),
        title_ptr_(nullptr),
        response_ptr_(new std::size_t{0uz})
    {
        if (!menu_items.empty())
            menu_init(menu_items[0].size());
    }


    /** @brief Sets title for menu and alignment. Default alignment for title is center */
    constexpr void title(const std::string& title, const Align alignment=Align::CENTER) noexcept {
        if (title_ptr_ != nullptr)
            title_ptr_->~Title_t();
        title_ptr_ = new Title_t{title};
        title_ptr_->set_align(alignment);
    }

    /** @returns title for menu or empty string if title not set */
    [[nodiscard]] constexpr std::string title() const noexcept { return title_ptr_->str(); }


    constexpr void clear_header() noexcept { header_ptr_->clear(); }


    /** @returns menu items from menu as vector */
    [[nodiscard]] constexpr str_vec_2d_t menu_items() const noexcept { return *menu_items_; }


    /**
     * assigns alignment to a desired column
     * @param col_index begins at 1 -> first column is 1begins at 1 -> first column is 1
     * @param alignment LEFT, RIGHT, CENTER
     * @param args more col_index and alignment arguments
     */
    template <typename... Args>
    constexpr void align(
        const std::size_t col_index,
        const Align alignment,
        Args&&... args) const {

        if (col_index > num_of_cols() || col_index < 1) {
            for (auto i{0uz}; i < num_of_cols(); ++i)
                col_chrs_ptr_[i].alignment = Align::LEFT;
            throw std::out_of_range(
                "Entered col index ->(" + std::to_string(col_index) +
                ") outside exceptable col range ->[1, " + std::to_string(num_of_cols()) + "]"
            );
        }

        col_chrs_ptr_[col_index - 1].alignment = alignment;

        if constexpr (sizeof...(args)) {
            align(std::forward<Args>(args)...);
        }
    }


    /**
     * Excludes specific rows from assigned column alignment. \n Resets on every call
     * @param rows list of rows to be excluded from column alignment by index. Will set to default alignment (LEFT)
     * @throws std::out_of_range if one of the entered rows is greater than the menu size or equals 0
     */
    constexpr void excl_align(const std::initializer_list<std::size_t>& rows) const {
        const auto size = this->size();
        for (const auto & row : rows)
            if (row > size || row == 0)
                throw std::out_of_range("Entered row index -> (" + std::to_string(row) +
                    ") outside acceptable range ->[1, " + std::to_string(size) + "]");
        Menu_Characteristics::row::exclude_from_alignment = rows;
    }


    /**
     * assigns alignment to a desired header
     * @param col_index begins at 1 -> first column is 1
     * @param alignment LEFT, RIGHT, CENTER
     * @param args
     */
    template <typename... Args>
    constexpr void align_header(const std::size_t col_index, const Align alignment, Args&&... args) const {
        if (col_index > num_of_cols() || col_index < 1)
            throw std::out_of_range(
                "Entered col index ->(" + std::to_string(col_index) +
                ") outside exceptable col range ->[1, " + std::to_string(num_of_cols()) + "]"
            );
        col_chrs_ptr_[col_index - 1].header_alignment = alignment;

        if constexpr (sizeof...(args))
            align_header(std::forward<Args>(args)...);
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
        if (empty())
            menu_init(row.size());

        if (row.size() != num_of_cols() && !empty())
            throw std::invalid_argument(
                "\nRow size ->(" + std::to_string(row.size()) + ") " +
                "does not match established number of columns ->(" + std::to_string(num_of_cols()) + ")"
            );

        menu_items_->emplace_back(row);

        if constexpr (sizeof...(rows) > 0)
            emplace_back(row);
    }


    /**
     * Appends rows to the menu
     * @param rows data to be appended to menu
     *
     * @throws std::invalid_argument If size of row != number of columns
     */
    constexpr void emplace_back(const std::initializer_list<std::vector<std::string>> rows)
    {
        if (empty())
            menu_init(rows.size());
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
    constexpr void headers(const std::vector<std::string>& headers) const {
        if (menu_items_->empty())
            throw std::runtime_error("\nMenu must have contents to add header");
        if (headers.size() != num_of_cols())
            throw std::invalid_argument("\nHeaders count mismatch");
        *header_ptr_ = headers;
        separators_->push_back({.index=1, .separator_char='-'});
    }


    /**
     * @brief sets headers for menu and automatically assigns the necessary separator
     *
     * @param header describes a column name
     * @param args more headers
     *
     * @throws std::invalid_argument if # of headers does not match # num of columns
     * @throws std::runtime_error if menu is not initialized
     */
    template <typename... Args>
    constexpr void add_headers(const std::string& header, Args&&... args)
    {
        headers(header, std::forward<Args>(args)...);

        if (header_ptr_->size() != num_of_cols()) {
            header_ptr_->clear();
            throw std::invalid_argument("\nHeader count mismatch");
        }

        separators_->push_back({.index=1, .separator_char='-'});
    }


    /**
     * @return headers for menu
     */
    [[nodiscard]] constexpr std::vector<std::string> headers() const noexcept { return *header_ptr_; }


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
        return menu_items_->at(0).size();
    }


    /**
     * Adds preceding dots to desired column
     * @param column begins at 1 <-> first column is 1
     * @param exclude_rows excludes desired row from having any proceeding dots. default is none
     * @throws std::out_of_range if entered column is not valid
     */
    constexpr void preceding_dots(
        const std::size_t column,
        const std::initializer_list<std::size_t>& exclude_rows={}) const {

        if (column < 1 || column > num_of_cols())
            throw std::out_of_range(
                "\nEntered column index -> (" + std::to_string(column) +
                ") not in acceptable range -> [1, " + std::to_string(num_of_cols()) + "]");
        col_chrs_ptr_[column - 1].preceding_dots = true;
        Menu_Characteristics::row::exclude_from_dots = exclude_rows;
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

        separators_->push_back({.index = index, .separator_char = sep_char});

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
            separators_->erase(separators_->begin() + (sep - separators_->data()));
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
    constexpr void response(const std::size_t resp) const {
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
        menu_items_->clear();
        separators_->clear();
        this->clear_header();
        *response_ptr_ = 0;

        if (title_ptr_ != nullptr) {
            delete title_ptr_;
        }
        title_ptr_ = nullptr;
    }


    /**
     * Clears response only
     */
    constexpr void reset_response() const noexcept { *response_ptr_ = 0; }


    /**
     * @brief Removes separator corresponding to search character
     * @param sep_char Character to be printed for seperator
     * @param args more character arguments
     */
    template <typename... Args>
    constexpr void remove_separators(const char sep_char, Args&&... args) noexcept {
        for (long long i{0}; i < separators_->size(); ++i) {
            if (separators_->at(i).separator_char == sep_char)
                separators_->erase(separators_->begin() + i);
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

    void menu_init(const std::size_t num_cols) noexcept {

        header_ptr_->reserve(num_cols);

        col_chrs_ptr_ = static_cast<Menu_Characteristics::column*>(
            malloc(num_cols * sizeof(Menu_Characteristics::column)));

        for (auto i{0uz}; i < num_cols; ++i) {
            new (&col_chrs_ptr_[i]) Menu_Characteristics::column();
        }
    }


    template <typename... Args>
    constexpr void headers(const std::string& header, Args&&... args) {
        if (menu_items_->empty())
            throw std::runtime_error("\nMenu must have contents to add header");

        header_ptr_->push_back(header);

        if constexpr (sizeof...(args) > 0)
            headers(std::forward<Args>(args)...);
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
        for (auto& item : *separators_) {
            if (item.index == row_index) return &item;
        }
        return nullptr;
    }

    /**
 * @memberof Menu<std::vector<std::vector<std::string>>>
 * @param helper pointer to print_helper object
 * @brief prints all rows of menu
 */
    constexpr void print_rows(const print_helper<str_vec_2d_t> *const helper)
    {
        if (!this->headers().empty()) this->print_header(helper);

        // Print Rows
        for (auto i{1uz}; i <= helper->items.size(); ++i) {
            this->print_separator(helper->width, i);
            std::cout << helper->build_row(i, this->col_chrs());
        }
    }

    /**
     * @brief prints title row. title is centered. If title has no value function just prints a newline
     * @param helper
     */
    void print_title(const print_helper<str_vec_2d_t> * helper) {
        switch (title_ptr_->get_align()) {
            case Align::LEFT:
                std::cout << "\n" << print_helper<std::vector<std::string>>::left(title_ptr_->str(), helper->width) << "\n";
                break;
            case Align::RIGHT:
                std::cout << "\n" << print_helper<std::vector<std::string>>::right(title_ptr_->str(), helper->width) << "\n";
                break;
            case Align::CENTER:
                std::cout << "\n" << print_helper<std::vector<std::string>>::center(title_ptr_->str(), helper->width) << "\n";
                break;
        }
    }


    /** @brief Prints the header row. Does not check if header has elements */
    constexpr void print_header(const print_helper<str_vec_2d_t> *const helper) {

        std::string row_str;
        const auto pad = new std::size_t{0uz};

        for (auto i{0uz}; i < helper->num_of_cols; ++i) {

            *pad = helper->col_dimensions_.at(i).buffer - header_ptr_->at(i).length();

            switch ((col_chrs_ptr_+ i)->header_alignment) {
                case Align::LEFT:
                    row_str.append(header_ptr_->at(i) + std::string(*pad, ' '));
                    break;
                case Align::RIGHT:
                    row_str.append(std::string(*pad, ' ') + header_ptr_->at(i));
                    break;
                case Align::CENTER:
                    const auto half_pad = *pad / 2;
                    const auto remainder = *pad % 2;
                    row_str.append(
                        std::string(half_pad, ' ') +
                        header_ptr_->at(i) +
                        std::string(half_pad + remainder, ' '));
                    break;
            }
        }
        delete pad;
        row_str.append("\n");
        std::cout << row_str;
    }


    [[nodiscard]] constexpr Menu_Characteristics::column* col_chrs() const {return col_chrs_ptr_;}



    str_vec_2d_t* menu_items_;
    std::vector<Separator>* separators_;
    std::vector<std::string>* header_ptr_;
    Menu_Characteristics::column* col_chrs_ptr_;
    Title_t* title_ptr_;
    std::size_t* response_ptr_;

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
        if (const auto h = menu->headers(); !h.empty())
            test_column.emplace_back(h.at(i));

        col_dimensions_.emplace_back(max_length_in_items(test_column) + 8, 0);
    }

    width = this->get_buffer_size(menu->title());
}


#endif // MENU_LIBRARY_H