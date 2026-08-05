#ifndef MENU_LIBRARY_H
#define MENU_LIBRARY_H
#include <cstdint>
#include <expected>
#include <string>
#include <optional>
#include <vector>
#include <iostream>

template <typename T>
concept is_2d_str_vec = std::is_same_v<std::decay_t<T>, std::vector<std::vector<std::string>>>;

template <typename T>
concept is_1d_str_vec = std::is_same_v<std::decay_t<T>, std::vector<std::string>>;

template <typename T>
concept is_menu_items = is_2d_str_vec<T> || is_1d_str_vec<T> ;


template <typename T>
concept StringLike = std::convertible_to<std::decay_t<T>, std::string>;

template <typename T> requires is_menu_items<T>
class Menu;


/**
 * Holds the characteristics of a seperator
 *
 * @tparam T required type is std::vector<std::string>> or std::vector<std::vector<std::string>>>.
 * Represents the two menu types
 */
template <typename T> requires is_menu_items<T>
struct Separator : Menu<T> {

    /**
     * @memberof Separator.
     * @brief Determines where to print separator.
     * Prints BEFORE index
     */
    std::size_t index;

    /** @memberof Separator. @brief character to be printed for separator */
    char separator_char;
};



enum class menu_error {
    none,
    header_count_mismatch,
};


/**
 * @brief Helper class which privately inherits the Menu class.
 * Does the calculations for the dimensions of the menu for printing
 * @tparam T required type is std::vector<std::string>> or std::vector<std::vector<std::string>>>.
 * Represents the two menu types
 */
template <typename T> requires is_menu_items<T>
class print_helper : Menu<T> {

public:

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
        for (const auto buffer : buffers) {
            total_buffer += buffer;
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

    /** @memberof print_helper. Refers to the max length an item may be in each column + a constant buffer */
    std::vector<std::size_t> buffers;

    /** @memberof print_helper. Refers to the buffer size - length of menu item for each column of a row */
    std::vector<uint8_t> padding;

    /** @memberof print_helper. Total width of the menu */
    std::size_t width;

    /** @memberof print_helper. Total number of columns */
    std::size_t num_of_cols;

};

template <typename T> requires is_menu_items<T>
class Menu {

public:

    // ~~~~~~~~~~~~ Constructors / Destructors ~~~~~~~~~~~~ //
    Menu() :
        menu_items_({}), header_({}), separators_({}) {}


    /**
     * @brief header and separators remain empty
     * @param menu_items container of strings to be the menu choices
     */
    explicit Menu(const T& menu_items) :
        menu_items_(menu_items), header_({}), separators_({}) {}


    /**
     * @brief automatically assigns separator for header
     *
     * @param menu_items container of strings to be the menu choices
     * @param header vector of strings to be headers
     *
     * @throws std::invalid_argument if the number of headers does not match the number of columns of menu.
     * headers and separators will remain empty
     */
    explicit Menu(const T& menu_items, const std::vector<std::string>& header) requires is_2d_str_vec<T> :
        menu_items_(menu_items), header_({}), separators_({})
    {
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
    template <StringLike string_l, typename... Args>
    explicit Menu(const string_l& header, Args&&... args, const T& menu_items)
    requires is_2d_str_vec<T> :
        menu_items_(menu_items), header_({}), separators_({})
    {
        try {
            this->headers(header, std::forward<Args>(args)...);
        }
        catch (const std::invalid_argument& e) {
            throw std::invalid_argument(e.what());
        }

    }

    ~Menu()=default;


    /** @brief Sets title for menu */
    template <StringLike S>
    constexpr void title(const S& title) noexcept { title_ = title; }


    /** @returns title for menu or empty string if title not set */
    [[nodiscard]] constexpr std::string title() const noexcept { return title_.value_or(""); }


    /**
     * @param or_val value to be returned if title is not set
     * @returns title for menu or the or_val
     */
    [[nodiscard]] constexpr std::string title(const std::string& or_val) const noexcept { return title_.value_or(or_val); }


    /** @brief sets menu items for menu */
    constexpr void menu_items(const T& menu_items) noexcept { menu_items_ = menu_items; }


    /** @returns menu items from menu as vector */
    [[nodiscard]] constexpr T menu_items() const noexcept { return menu_items_; }


    /**
     * @brief sets headers for menu c
     * @param headers describes a vector of column names
     *
     * @throws std::invalid_argument if the number of headers does not match the number of columns of menu.
     */
    template <StringLike S>
    constexpr void headers(const std::vector<S>& headers) noexcept requires is_2d_str_vec<T>
    {
        if (headers.size() != menu_items_[0].size()) {
            throw std::invalid_argument("\nHeaders count mismatch");
        }
        header_ = headers;
        separators_.emplace_back({.index=1, .separator_char='-'});
    }


    /**
     * @brief sets headers for menu and automatically assigns the necessary separator
     *
     * @param header describes a column name
     * @param args more headers
     *
     * @throws std::invalid_argument if # of headers does not match # num of columns
     */
    template <StringLike string_l, typename... Args>
    constexpr void headers(const string_l& header, Args&&... args) noexcept
    requires is_2d_str_vec<T>
    {
        header_.emplace_back(header);
        if constexpr (sizeof...(args) > 0) {
            headers(std::forward<Args>(args)...);
        }

        if (header_.size() != menu_items_[0].size()) {
            header_ = {};
            throw std::invalid_argument("\nHeader count mismatch");
        }

        separators_.push_back({.index=1, .separator_char='-'});
    }


    /**
     * @return headers for menu
     */
    [[nodiscard]] constexpr std::vector<std::string> headers() const noexcept { return header_; }


    /**
     * @return number of menu items
     */
    [[nodiscard]] constexpr auto size() const noexcept { return menu_items_.size(); }


    /**
     * @brief Adds separators to menu. prints across entire width of menu
     *
     * @param sep_char Character to be printed for seperator
     * @param index Determines where to print separator. Prints BEFORE index
     * @param args more arguments for index and character
     *
     * @throws std::invalid_argument If a seperator already exist for an inputted index
     * @throws std::invalid_argument If index is larger than menu size or index is 1
     */
    template <typename... Args>
    constexpr void separators(const char sep_char, const std::size_t index, Args&&... args) {

        if (this->find_separator(index) != nullptr)
            throw std::invalid_argument("\nSeperator already exists for: " + std::to_string(index));

        if (index > menu_items_.size() || index <= 1)
            throw std::invalid_argument(
                "\nEntered index ->(" + std::to_string(index) + ") outside acceptable range " +
                "[2, menu_size->(" + std::to_string(menu_items_.size()) + ")]"
            );

        separators_.push_back({.index = index, .separator_char = sep_char});

        if constexpr (sizeof...(args) > 0) {
            separators(std::forward<Args>(args)...);
        }
    }


    /**
     * @brief removes separator at desired index
     * @param index Determines location of separator.
     * @param args more arguments for index
     */
    template <typename... Args>
    constexpr void remove_separator_at(const std::size_t index, Args&&... args) noexcept
    {
        if (auto sep = this->find_separator(index); sep != nullptr)
            separators_.erase(separators_.begin() + (sep - separators_.data()));

        if constexpr (sizeof...(args) > 0)
            separators(std::forward<Args>(args)...);
    }


    /**
     * @brief Sets user response to menu
     * @param resp response in the form of an integer corresponding to selection
     *
     * @throws std::invalid_argument If response is less than 1 or greater than size of menu
     */
    constexpr void response(const std::size_t resp) {

        if (resp < 1 || resp > menu_items_.size())
            throw std::invalid_argument(
                "\nEntered response ->(" + std::to_string(resp) + ") outside acceptable range " +
                "[1, menu_size->(" + std::to_string(menu_items_.size()) + ")]"
            );

        response_ = resp;
    }

    /**
     * @brief Sets user response to menu. do not include numbered index
     * @param resp response in the form of a string corresponding to selection
     * @note Does not check if there is multiple of the same response options. Will set index of first found only
     * @throws std::invalid_argument If argument does not match any menu choices
     */
    template <StringLike string_l>
    constexpr void response(const string_l& resp) requires is_1d_str_vec<T> {

        const auto search_resp = this->find_menu_item(resp);

        if (search_resp == nullptr) {
            throw std::invalid_argument("\nEntered Response ->(" + std::string(resp) + ") Not Found " );
        }

        response_ = menu_items_.begin() + (search_resp - menu_items_.data());
    }


    /**
     * @throws std::runtime_error if response has not been set
     * @return response as integer
     */
    [[nodiscard]] constexpr std::size_t response() const {

        if (response_.has_value())
            return response_.value();

        throw std::runtime_error("\nResponse not set");
    }


    /**
     * @throws std::runtime_error if response has not been set
     * @return response as the corresponding menu item string
     */
    [[nodiscard]] constexpr std::string response_as_menu_item() const requires is_1d_str_vec<T> {

        if (response_.has_value())
            return menu_items_[response_.value() - 1];

        throw std::runtime_error("\nResponse not set");
    }


    /**
     * Clears all menu items, all separators, title, header(if applicable), and response
     */
    constexpr void reset() noexcept {
        menu_items_.clear();
        separators_.clear();
        header_.clear();
        response_ = std::nullopt;
        title_ = std::nullopt;
    }


    /**
     * Clears response only
     */
    constexpr void reset_response() noexcept { response_ = std::nullopt; }


    /**
     * @brief Removes separator corresponding to search character
     * @param sep_char Character to be printed for seperator
     * @param args more character arguments
     */
    template <typename... Args>
    constexpr void remove_separators(const char sep_char, Args&&... args) noexcept
    {
        for (auto i{0uz}; i < separators_.size(); ++i) {
            if (separators_.at(i).separator_char == sep_char)
                separators_.erase(separators_.begin() + i);
        }

        if constexpr (sizeof...(args) > 0) {
            separators(std::forward<Args>(args)...);
        }
    }


    /**
 * @memberof Menu
 * @brief prints entire menu include title, separators, and rows containing the menu items
 * @note separators are printed BEFORE the entered index
 */
    constexpr void print();


private:

    /**
     * @memberof Menu
     * @param width total num of chars to print for separator
     * @param row_index separator is printed just before this row
     * @brief prints separator
     */
    void print_separator(const std::size_t width, const std::size_t row_index) {
        const Separator<T> *const sep = this->find_separator(row_index);
        if (sep == nullptr) return;
        std::cout << std::string(width, sep->separator_char) << "\n";
    }

    /**
     * @param row_index used to identify Separator struct
     * @return pointer to separator struct for passed index, else nullptr if not found
     */
    Separator<T>* find_separator(const std::size_t row_index) noexcept {
        for (auto& item : separators_) {
            if (item.index == row_index) return &item;
        }
        return nullptr;
    }

    /** @brief Prints all rows of menu */
    constexpr void print_rows(print_helper<T> * helper);

    /**
     * @brief prints title row. title is centered. If title has no value function just prints a newline
     * @param helper
     */
    void print_title(print_helper<T> * helper) {
        // Print title if title is assigned a value
        if (title_.has_value()) {
            std::cout
                << "\n"
                << print_helper<std::vector<std::string>>::center(title_.value(), helper->width)
                << "\n";
        }
        else std::cout << "\n";
    }


    /** @brief Prints the header row. Does not check if header has elements */
    constexpr void print_header(const print_helper<T> *const helper) {
        std::string row_str;
        /// for each column
        for (auto i{0uz}; i < helper->num_of_cols; ++i) {
            /// build string for row with item at [row - 1, col] + padding for column
            row_str.append(header_.at(i) + std::string(helper->buffers.at(i) - header_.at(i).length(), ' '));
        }
        row_str.append("\n");
        std::cout << row_str;
    }


    /**
 * @memberof Menu
 * @param helper pointer to print_helper class
 * @brief prints separator for header or footer (same length)
 */
    void print_separator(const print_helper<T> *const helper) const {
        std::cout << std::string(helper->width, '~') << "\n";
    }


    template <StringLike string_l>
    constexpr std::string* find_menu_item(const string_l& search_item) const {
        for (const auto item : menu_items_) {
            if (item == search_item)
                return &item;
        }
        return nullptr;
    }


    T menu_items_;
    std::vector<std::string> header_;
    std::vector<Separator<T>> separators_;
    std::optional<std::size_t> response_;
    std::optional<std::string> title_;

};

template <>
print_helper<std::vector<std::vector<std::string>>>::print_helper(const Menu * menu);
template <>
print_helper<std::vector<std::string>>::print_helper(const Menu * menu);
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
    for (uint8_t i = 0; i < num_of_cols; ++i) {
        /// If the index is less than the columns size
        if (row_index - 1 < items.at(i).size()) {
            /// build string for row with item at [col, row - 1] + padding for column
            row_str.append(items.at(i).at(row_index-1) + std::string(padding.at(i), ' '));
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
    const std::size_t row_index)
const {
    std::string row_str;
    /// for each column
    for (uint8_t i = 0; i < num_of_cols; ++i) {
        /// build string for row with item at [row - 1, col] + padding for column
        row_str.append(items.at(row_index-1).at(i) + std::string(padding.at(i), ' '));
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
    padding = {};
    /// for each column
    for (uint8_t i = 0; i < num_of_cols; ++i) {
        /// if not the last column
        if (i < num_of_cols - 1) {
            /// if row index is less than or equal to the size of the column
            if (row_index <= items.at(i).size()) {
                /// padding = buffer - length of menu item
                padding.emplace_back(buffers.at(i) - items.at(i).at(row_index-1).length());
            }
            else {
                padding.emplace_back(0);
            }
        }
        /// if the last column
        else if (i == num_of_cols - 1) {
            padding.emplace_back(0);
        }
    }
}
/**
 * @overload void set_padding(const std::size_t row_index)
 * @memberof print_helper<std::vector<std::vector<std::string>>>
 */
template <>
inline void print_helper<std::vector<std::vector<std::string>>>::set_padding(const std::size_t row_index) {
    padding = {};
    /// for each column
    for (uint8_t i = 0; i < num_of_cols; ++i) {
        /// if not the last column
        if (i < num_of_cols - 1) {
            /// padding = buffer - length of menu item
            padding.emplace_back(buffers.at(i) - items.at(row_index-1).at(i).length());
        }
        /// if the last column
        else if (i == num_of_cols - 1) {
            padding.emplace_back(0);
        }
    }
}
/**
 * @memberof Menu<std::vector<std::vector<std::string>>>
 * @param helper pointer to print_helper object
 * @brief prints all rows of menu
 */
template <>
constexpr void Menu<std::vector<std::vector<std::string>>>::print_rows(
    print_helper<std::vector<std::vector<std::string>>> *const helper)
{
    if (this->headers().size()) this->print_header(helper);

    // Print Rows
    for (auto i{1uz}; i <= helper->items.size(); ++i) {
        this->print_separator(helper->width, i);
        helper->set_padding(i);
        std::cout << helper->build_row(i);
    }
}
/**
 * @memberof Menu<std::vector<std::string>>
 * @param helper pointer to print_helper object
 * @brief prints all rows of menu
 */
template <>
constexpr void Menu<std::vector<std::string>>::print_rows(
    print_helper<std::vector<std::string>> *const helper)
{
    if (helper->num_of_cols == 1) {
        for (uint8_t i{0}; i < static_cast<uint8_t>(this->size()); ++i) {
            this->print_separator(helper->width, i + 1);
            std::cout << i + 1 << ") " << menu_items_[i] << "\n";
        }
    }
    else {
        // Print Rows
        for (auto i{1uz}; i <= helper->items[0].size(); ++i) {
            this->print_separator(helper->width, i);
            helper->set_padding(i);
            std::cout << helper->build_row(i);
        }
    }
}

/** @overload constexpr void print() */
template <typename T> requires is_menu_items<T>
constexpr void Menu<T>::print() {
    print_helper print_helper_(this);

    this->print_title(&print_helper_);
    this->print_separator(&print_helper_);
    this->print_rows(&print_helper_);
    this->print_separator(&print_helper_);
}



template <>
inline print_helper<std::vector<std::string>>::print_helper(const Menu * const menu)  {

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
    for (uint8_t i = 0; i < num_of_cols; ++i) {
        if (i==0)
            it_positions.emplace_back(slice + (remainder > i));
        else
            it_positions.emplace_back(it_positions.at(i - 1) + slice + (remainder > i));
    }


    if (num_of_cols >= 2) {

        // create the first column and calculate buffer
        items.emplace_back(menu_items.begin(), menu_items.begin() + it_positions[0]);
        buffers.emplace_back(max_length_in_items(items[0]) + 8);

        /*
         * This loop only applies if num_of_cols >= 3.
         * creates all columns that are between the first and the last and calculates buffers
         */
        for (uint8_t i = 0; i < num_of_cols - 2; ++i) {
            items.emplace_back(menu_items.begin() + it_positions[i], menu_items.begin() + it_positions[i+1]);
            buffers.emplace_back(max_length_in_items(items[i + 1]) + 8);
        }

        // create the last column. buffer not necessary
        items.emplace_back(menu_items.begin() + it_positions[num_of_cols - 2], menu_items.end());

        width = this->get_buffer_size(menu->title(), max_length_in_items(items[num_of_cols - 1]));
    }
    else
        width = this->get_buffer_size(menu->title(), max_length_in_items(menu_items));
}




template <>
inline print_helper<std::vector<std::vector<std::string>>>::print_helper(const Menu * const menu)  {

    items = menu->menu_items();
    num_of_cols = items.at(0).size();

    // adding numbered index to menu items in first column, format: #) menu_item[0]
    for (auto i{0uz}; i < items.size(); ++i) {
        items.at(i).at(0).insert(0, std::to_string(i + 1) + ") ");
    }

    for (auto i{0uz}; i < num_of_cols; ++i) {

        auto test_column = get_col(items, i);

        // if there are headers then they need to be included in buffer calculation
        if (const auto h = menu->headers(); h.size())
            test_column.emplace_back(h.at(i));

        // Buffer is not necessary for last column
        if (i == num_of_cols - 1) {
            buffers.emplace_back(max_length_in_items(test_column));
        }
        else
            buffers.emplace_back(max_length_in_items(test_column) + 8);
    }

    width = this->get_buffer_size(menu->title());
}


#endif // MENU_LIBRARY_H