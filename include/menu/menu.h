#ifndef MENU_LIBRARY_H
#define MENU_LIBRARY_H
#include "menu_types.hpp"
#include "Title.hpp"
#include "Columns.hpp"
#include "Separators.hpp"
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

enum class menu_error_t {
    none,
    header_count_mismatch,
    response_already_set,
    not_in_range,
    uninitialized
};

namespace Menu_Characteristics {
    struct row {
        inline static std::initializer_list<std::size_t> exclude_from_dots;
        inline static std::initializer_list<std::size_t> exclude_from_alignment;

    };
}

static constexpr uint8_t FIRST_COLUMN = 0;


template <>
class Menu<SINGLE_COLUMN> {
protected:
    struct Dimensions {
        /**
     * @memberof print_helper<std::vector<std::string>>
     * @param title used for comparison to the buffer size
     * @param additional added to total buffer size
     * @returns max(total buffer size , length of title)
     */
        [[nodiscard]] constexpr std::size_t get_buffer_size(const std::string& title, const std::size_t additional=0) const {
            auto total_buffer{0uz};
            for (const auto& col : col_dimensions) {
                total_buffer += col.buffer;
            }
            total_buffer += additional;
            return std::max(total_buffer, title.length());
        }
        /** @memberof print_helper. Stores the menu items */
        std::vector<std::vector<std::string>> items;
        std::vector<col_dimensions> col_dimensions;
        /** @memberof print_helper. Total width of the menu */
        std::size_t width;
        std::size_t num_of_cols;
    };
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
        menu_items_(new str_vec_1d_t{menu_items}), title_(nullptr), response_(new std::size_t{0}) {}

    /**
     * Initializes menu if argument is not empty
     * @brief response remains empty
     * @param menu_items container of strings to be the menu choices
     * @param title value to be title
     */
    explicit Menu(const str_vec_1d_t &menu_items, const std::string &title) :
        menu_items_(new str_vec_1d_t{menu_items}),title_(new Title_t(title.data())), response_(new std::size_t{0}) {}

    /** @brief Sets title for menu and alignment. Default alignment for title is center */
    constexpr void title(
        const std::string& title,
        const Align alignment=Align::CENTER,
        const Color color=Color::WHITE ,
        const Style style=Style::NONE ) noexcept
    {
        title_ = new Title_t{title.c_str(), alignment, color, style};
    }

    /** @returns title for menu */
    [[nodiscard]] constexpr std::string title() const noexcept { return title_->c_str(); }


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
        delete title_;
        title_ = nullptr;
    }

    /**
     * Clears response only
     */
    constexpr void reset_response() noexcept { *response_ = 0; }

    /** @overload constexpr void print() */
    constexpr void print() {
        Dimensions dimensions = calculate_dimensions();
        title_->print(dimensions.width);
        std::cout << std::string(dimensions.width, '~') << "\n";
        print_rows(&dimensions);
        std::cout << std::string(dimensions.width, '~') << "\n";
    }

private:

    str_vec_1d_t* menu_items_;
    Title_t* title_;
    std::size_t* response_;


    constexpr Dimensions calculate_dimensions() const {
        auto menu_items = this->menu_items();
        auto slice{0uz};
        uint8_t remainder{0};
        const auto size = menu_items.size();
        std::vector<std::size_t> it_positions;
        Dimensions r_dimensions;


        // adding numbered index to all menu items, format: #) menu_item
        for (auto i{0uz}; i < menu_items.size(); ++i) {
            menu_items.at(i).insert(0, std::to_string(i + 1) + ") ");
        }

        r_dimensions.num_of_cols = get_num_of_cols(size);

        // slice is used to determine the interval of indexes to make into individual columns
        slice = size / r_dimensions.num_of_cols;

        /*
         *  remainder determines if a column needs an extra spot assuming the number of
         *  elements in menu is not dividable by the number of columns
         */
        remainder = size % r_dimensions.num_of_cols;


        // Calculate the positions for the slicing for the columns
        for (auto i{0uz}; i < r_dimensions.num_of_cols; ++i) {
            if (i==0)
                it_positions.emplace_back(slice + (remainder > i));
            else
                it_positions.emplace_back(it_positions.at(i - 1) + slice + (remainder > i));
        }


        if (r_dimensions.num_of_cols > 1) {

            // create the first column and calculate buffer
            r_dimensions.items.emplace_back(menu_items.begin(), menu_items.begin() + it_positions[0]);
            r_dimensions.col_dimensions.emplace_back(max_length_in_items(r_dimensions.items[0]) + 8, 0);

            /*
             * This loop only applies if num_of_cols >= 3.
             * creates all columns that are between the first and the last and calculates buffers
             */
            for (uint8_t i = 0; i < r_dimensions.num_of_cols - 2; ++i) {
                r_dimensions.items.emplace_back(menu_items.begin() + it_positions[i], menu_items.begin() + it_positions[i+1]);
                r_dimensions.col_dimensions.emplace_back(max_length_in_items(r_dimensions.items[i + 1]) + 8, 0);
            }

            // create the last column. buffer not necessary
            r_dimensions.items.emplace_back(menu_items.begin() + it_positions[r_dimensions.num_of_cols - 2], menu_items.end());
            r_dimensions.col_dimensions.emplace_back(max_length_in_items(r_dimensions.items[r_dimensions.num_of_cols - 1]), 0);

            r_dimensions.width = r_dimensions.get_buffer_size(this->title());
        }
        else
            r_dimensions.width = r_dimensions.get_buffer_size(this->title(), max_length_in_items(menu_items));
        return r_dimensions;
    }

    /**
 * @memberof print_helper
 * @param menu_size size of menu in terms of number of menu items in vector
 * @returns number of columns necessary for the size of menu. range is [1,5]
 */
    static constexpr uint8_t get_num_of_cols(const std::size_t menu_size) noexcept {
        if (menu_size >= 100) return 5;
        if (menu_size >= 60)  return 4;
        if (menu_size >= 30)  return 3;
        if (menu_size >= 10)  return 2;
        return 1;
    }

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
    constexpr void print_rows( Dimensions *const dimensions_ptr ) {
        if (dimensions_ptr->num_of_cols == 1) {
            for (uint8_t i{0}; i < static_cast<uint8_t>(this->size()); ++i) {
                std::cout << i + 1 << ") " << menu_items_->at(i) << "\n";
            }
        }
        else {
            // Print Rows
            for (auto i{1uz}; i <= dimensions_ptr->items[0].size(); ++i) {
                set_padding(i, dimensions_ptr);
                std::cout << build_row(i, dimensions_ptr);
            }
        }
    }

    /**
* @overload std::string build_row(const std::size_t row_index) const
* @memberof print_helper<std::vector<std::string>>
 */
    constexpr std::string build_row(const std::size_t row_index, const Dimensions *const dimensions_ptr) const {
        std::string row_str;
        /// for each column
        for (auto i{0uz}; i < dimensions_ptr->num_of_cols; ++i) {
            /// If the index is less than the columns size
            if (row_index - 1 < dimensions_ptr->items.at(i).size()) {
                /// build string for row with item at [col, row - 1] + padding for column
                row_str.append(dimensions_ptr->items.at(i).at(row_index-1) +
                    std::string(dimensions_ptr->col_dimensions.at(i).padding, ' '));
            }
        }
        row_str.append("\n");
        return row_str;
    }
    /**
 * @overload void set_padding(const std::size_t row_index)
 * @memberof print_helper<std::vector<std::string>>
 */
    constexpr void set_padding(const std::size_t row_index, Dimensions *const dimensions_ptr) {
        /// for each column
        for (auto i{0uz}; i < dimensions_ptr->num_of_cols; ++i) {

            /// if not the last column
            if (i < dimensions_ptr->num_of_cols - 1) {
                /// if row index is less than or equal to the size of the column
                if (row_index <= dimensions_ptr->items.at(i).size()) {
                    /// padding = buffer - length of menu item
                    dimensions_ptr->col_dimensions.at(i).padding =
                        dimensions_ptr->col_dimensions.at(i).buffer - dimensions_ptr->items.at(i).at(row_index-1).length();
                }
                else {
                    dimensions_ptr->col_dimensions.at(i).padding = 0;
                }
            }
            /// if the last column
            else if (i == dimensions_ptr->num_of_cols - 1) {
                dimensions_ptr->col_dimensions.at(i).padding = 0;
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
template <std::size_t columns>
class Menu {

protected:
    class print_helper;
public:
    /**
     * Destructor -- Frees up all previously allocated memory
     */
    ~Menu() {
        free_all(*this);
    }


    /**
     * Copy Constructor \n Creates new copy of a Menu object
     * @param other any other Menu object
     */
    Menu(const Menu& other) :
        menu_items_{other.menu_items_},
        separators_{new Separator{*other.separators_}},
        columns_ptr_{new Columns<columns>{*other.columns_ptr_}},
        title_ptr_{nullptr},
        response_ptr_{new std::size_t{*other.response_ptr_}},
        color_conditions_{other.color_conditions_}
    {
        if (other.title_ptr_ != nullptr)
            title_ptr_ = new Title_t{*other.title_ptr_};
    }


    /**
     * Move Constructor \n Transfers ownership of memory and nulls other object
     * @param other any other Menu object
     */
    Menu(Menu&& other) noexcept :
        menu_items_{other.menu_items_},
        separators_{other.separators_},
        columns_ptr_{other.columns_ptr_},
        title_ptr_{other.title_ptr_},
        response_ptr_{other.response_ptr_},
        color_conditions_{other.color_conditions_} { null_all(other); }


    /**
     * Copy Assignment \n Frees up LHS existing memory and makes a copy of RHS Menu object memory
     * @param other any other Menu object
     * @return Copy of RHS Menu object
     */
    Menu& operator=(const Menu& other) {
        if (this == &other)
            return *this;

        const auto new_menu_items = other.menu_items_;
        const auto new_separators = new Separator{*other.separators_};
        const auto new_columns_ptr = new Columns<columns>{*other.columns_ptr_};
        const auto new_response_ptr = new std::size_t{*other.response_ptr_};

        Title_t* new_title_ptr;
        if (other.title_ptr_ != nullptr)
            new_title_ptr = new Title_t{*other.title_ptr_};
        else
            new_title_ptr = nullptr;

        // Free up existing memory
        free_all(*this);

        // Copy
        menu_items_ = new_menu_items;
        separators_ = new_separators;
        columns_ptr_ = new_columns_ptr;
        response_ptr_ = new_response_ptr;
        title_ptr_ = new_title_ptr;
        color_conditions_ = other.color_conditions_;

        return *this;
    }


    /**
     * Move Assignment \n Releases LHS memory and steals RHS memory
     * @param other any other Menu object
     * @returns LHS with all of RHS memory and leaves RHS NULL
     */
    Menu& operator=(Menu&& other) noexcept {
        if (this == &other)
            return *this;

        // Free existing memory
        free_all(*this);

        // Copy other memory
        menu_items_ = other.menu_items_;
        separators_ = other.separators_;
        columns_ptr_ = other.columns_ptr_;
        title_ptr_ = other.title_ptr_;
        response_ptr_ = other.response_ptr_;
        color_conditions_ = other.color_conditions_;

        // NULL other memory
       null_all(other);

        return *this;
    }

    Menu() :
        separators_(new Separator{}),
        columns_ptr_(new Columns<columns>{}),
        title_ptr_(nullptr),
        response_ptr_(new std::size_t{0uz}) { menu_items_.status_ = Menu_Items_t::Uninitialized; }

    /**
     * @brief title, header, and separators remain empty
     * @param menu_items container of strings to be the menu choices
     */
    explicit Menu(const str_vec_2d_t& menu_items) :
        separators_(new Separator{}),
        columns_ptr_(new Columns<columns>{}),
        title_ptr_(nullptr),
        response_ptr_(new std::size_t{0uz}) { init_menu_items(menu_items); }


    /** @brief Sets title for menu and alignment. Default alignment for title is center */
    constexpr void set_title(
        const std::string& title,
        const Align alignment=Align::CENTER,
        const Color color=Color::WHITE ,
        const Style style=Style::NONE ) noexcept
    {
        title_ptr_ = new Title_t{title.c_str(), alignment, color, style};
    }

    /** @returns title for menu or empty string if title not set */
    [[nodiscard]] constexpr std::string get_title() const noexcept
        { return title_ptr_==nullptr ? "" : title_ptr_->c_str(); }


    constexpr void clear_header() noexcept { columns_ptr_->clear(); }


    /** @returns menu items from menu as vector */
    [[nodiscard]] constexpr str_vec_2d_t get_menu_items() const noexcept { return menu_items_.items_; }

    constexpr void set_menu_items(const str_vec_2d_t& menu_items) noexcept { init_menu_items(menu_items); }


    /**
     * assigns alignment to a desired column
     * @param col_index begins at 1 -> first column is 1begins at 1 -> first column is 1
     * @param alignment LEFT, RIGHT, CENTER
     * @param args more col_index and alignment arguments
     * @throws std::runtime_error if menu is uninitialized
     */
    template <typename... Args>
    constexpr void align_column(const std::size_t col_index, const Align alignment, Args&&... args) const {
        if (menu_items_.status_ != Menu_Items_t::NonEmpty)
            throw std::runtime_error("Menu must be initialized and non-empty to adjust columns");
        columns_ptr_->align_column(col_index, alignment, args...);
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
     * Appends a row to the menu
     * @param row data to be appended to menu
     * @param rows additional rows
     *
     * @throws std::invalid_argument If size of row != number of columns
     */
    template <typename... Args>
    constexpr void add_rows(const std::vector<std::string>& row, const Args&... rows)
    requires (std::same_as<std::remove_cvref_t<Args>, std::vector<std::string>> && ...)
    {
        if (row.size() != columns)
            throw std::invalid_argument(
                "\nRow size ->(" + std::to_string(row.size()) + ") " +
                "does not match established number of columns ->(" + std::to_string(columns) + ")"
            );
        menu_items_.items_.emplace_back(row);
        if constexpr (sizeof...(rows) > 0)
            add_rows(row);
    }


    /**
     * Appends rows to the menu
     * @param rows data to be appended to menu
     *
     * @throws std::invalid_argument If size of row != number of columns
     */
    constexpr void add_rows(const std::initializer_list<std::vector<std::string>> rows) {
        for (const auto & row : rows)
            add_rows(row);
    }


    /**
     * @brief sets headers for menu c
     * @param headers describes a vector of column names
     *
     * @throws std::invalid_argument if the number of headers does not match the number of columns of menu.
     * @throws std::runtime_error If menu is empty or uninitialized
     */
    constexpr void headers(const std::initializer_list<std::string>& headers) const {
        if (menu_items_.status_ != Menu_Items_t::NonEmpty)
            throw std::runtime_error("Menu must be initialized and non-empty to set column names");
        columns_ptr_->set_headers(headers);
    }


    /**
     * @brief sets headers for menu and automatically assigns the necessary separator
     *
     * @param headers list of headers
     *
     * @throws std::invalid_argument if # of headers is greater than # of columns
     */
    constexpr void headers(std::ranges::input_range auto&& headers){
        columns_ptr_->set_headers(std::forward<decltype(headers)>(headers));
    }


    constexpr void style_header(
        const std::size_t col_index,
        const Align alignment=Align::LEFT,
        const Color color=Color::WHITE,
        const Style style=Style::NONE)
    {
        if (menu_items_.status_ != Menu_Items_t::NonEmpty)
            throw std::runtime_error("Menu must be initialized and non-empty to adjust columns");
        if (col_index > columns || col_index == 0)
            throw std::out_of_range(
                "Entered col index ->(" + std::to_string(col_index) +
                ") outside exceptable col range ->[1, " + std::to_string(columns) + "]"
            );
        columns_ptr_->style_headers(col_index - 1, alignment, color, style);
    }


    /**
     *  Column indexing starts at 1
     * @return header for menu
     */
    [[nodiscard]] constexpr std::string get_header(std::size_t col_index) const noexcept {
        return columns_ptr_->get_header(col_index);
    }


    /**
     * @return number of menu items
     */
    [[nodiscard]] constexpr std::size_t size() const noexcept { return menu_items_.items_.size(); }


    /**
     * @return true if menu is empty or uninitialized, false O.W.
     */
    [[nodiscard]] constexpr bool empty() const noexcept {
        return menu_items_.status_ == Menu_Items_t::Empty || menu_items_.status_ == Menu_Items_t::Uninitialized;
    }


    /**
     * Adds preceding dots to desired column
     * @param column begins at 1 <-> first column is 1
     * @param exclude_rows excludes desired row from having any proceeding dots. default is none
     * @throws std::out_of_range if entered column is not valid
     */
    constexpr void preceding_dots(
        const std::size_t column,
        const std::initializer_list<std::size_t>& exclude_rows={}) const
    {
        if (menu_items_.status_ != Menu_Items_t::NonEmpty)
            throw std::runtime_error("Menu must be initialized and non-empty to adjust columns");
        columns_ptr_->preceding_dots(column);
        Menu_Characteristics::row::exclude_from_dots = exclude_rows;
    }


    /**
     * @brief Adds separators to menu. prints across entire width of menu.
     * If a separator already exists at specified index, function returns without assigning separator
     *
     * @param sep_char Character to be printed for seperator
     * @param index Determines where to print separator. Prints BEFORE index
     * @param color
     * @param args more arguments for index and character
     *
     * @throws std::invalid_argument If index is larger than menu size or index is less than 1
     */
    template <typename... Args>
    constexpr void separators(
        const char sep_char,
        const std::size_t index,
        const Color color=Color::WHITE,
        Args&&... args)
    {
        if (separators_->find_separator(index) != nullptr)
            return;

        if (index > menu_items_.items_.size() || index < 1)
            throw std::invalid_argument(
                "\nEntered index ->(" + std::to_string(index) + ") outside acceptable range " +
                "[1, menu_size->(" + std::to_string(menu_items_.items_.size()) + ")]"
            );

        separators_->add(sep_char, color, index);

        if constexpr (sizeof...(args) > 0)
            separators(std::forward<Args>(args)...);
    }


    /**
     * @brief removes separator at desired index
     * @param index Determines location of separator.
     * @param args more arguments for index
     */
    template <typename... Args>
    constexpr void remove_separator_at(const std::size_t index, Args&&... args) noexcept{
        separators_->remove_separator_at(index, args...);
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
        if (resp < 1 || resp > menu_items_.items_.size())
            throw std::invalid_argument(
                "\nEntered response ->(" + std::to_string(resp) + ") outside acceptable range " +
                "[1, menu_size->(" + std::to_string(menu_items_.items_.size()) + ")]"
            );
        *response_ptr_ = resp;
    }

    /**
     * @throws std::runtime_error if response has not been set
     * @return response as integer
     */
    [[nodiscard]] constexpr std::size_t get_response() const {
        if (*response_ptr_ != 0)
            return *response_ptr_;
        throw std::runtime_error("\nResponse not set");
    }


    /**
     * Clears all menu items, all separators, title, header(if applicable), and response
     */
    constexpr void reset() noexcept {
        menu_items_.items_.clear();
        menu_items_.status_ = Menu_Items_t::Uninitialized;
        separators_->clear();
        this->clear_header();
        *response_ptr_ = 0;
        delete title_ptr_;
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
        separators_->remove_separator(sep_char);
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
        if (title_ptr_ != nullptr)
            title_ptr_->print(print_helper_.width);
        std::cout << std::string(print_helper_.width, '~') << "\n";
        if (menu_items_.status_ == Menu_Items_t::NonEmpty)
            this->print_rows(&print_helper_);
        std::cout << std::string(print_helper_.width, '~') << "\n";
    }



    constexpr std::size_t total_lines_for_print() const noexcept {
        // Top and bottom border + newline
        auto tlp{3uz};
        if (title_ptr_ != nullptr)
            tlp++;
        // headers
        if (!headers().empty())
            tlp++;
        return tlp + separators_->count() + size();
    }


    constexpr void color_element_if_else(
        const std::size_t col_index_to_color,
        const Color color,
        const std::size_t col_index_for_condition,
        const std::string& equal_condition,
        const Color else_color=Color::WHITE)
    {
        if (col_index_to_color > columns || col_index_to_color < 1)
            throw std::invalid_argument("Entered col index ->(" + std::to_string(col_index_to_color) +
                ") outside exceptable col range ->[1, " + std::to_string(columns) + "]");
        if (col_index_for_condition > columns || col_index_for_condition < 1)
            throw std::invalid_argument("Entered col index ->(" + std::to_string(col_index_for_condition) +
                ") outside exceptable col range ->[1, " + std::to_string(columns) + "]");
        if (col_index_to_color == col_index_for_condition)
            throw std::invalid_argument("Parameters for column indexes cannot be equal");
        color_conditions_.conditions_.emplace_back(Color_Condition_s{
            .col_index_to_color_ = col_index_to_color,
            .color_ = color,
            .col_index_for_condition_ = col_index_for_condition,
            .equal_condition_ = equal_condition,
            .else_color_ = else_color});
    }


private:

    static constexpr void free_all(Menu& menu) noexcept {
        delete menu.response_ptr_;
        delete menu.separators_;
        delete menu.columns_ptr_;
        delete menu.title_ptr_;
    }

    static constexpr void null_all(Menu& menu) noexcept {
        menu.response_ptr_ = nullptr;
        menu.separators_ = nullptr;
        menu.columns_ptr_ = nullptr;
        menu.title_ptr_ = nullptr;
    }

    constexpr void init_menu_items(const str_vec_2d_t& menu_items) {
        if (menu_items.empty()) {
            menu_items_.status_ = Menu_Items_t::Empty;
        }
        else if (menu_items[0].size() != columns)
            throw std::length_error{"Cannot construct Menu: Number of columns ->(" +
                std::to_string(menu_items[0].size()) + ") " +
                "does not match established number of columns ->(" + std::to_string(columns) + ")"};
        else {
            menu_items_.items_ = menu_items;
            menu_items_.status_ = Menu_Items_t::NonEmpty;
        }
    }



    /**
 * @memberof Menu<std::vector<std::vector<std::string>>>
 * @param helper pointer to print_helper object
 * @brief prints all rows of menu
 */
    constexpr void print_rows(const print_helper *const helper) {
        if (!columns_ptr_->empty())
            columns_ptr_->print_header(helper->col_dimensions_);
        // Print Rows
        for (auto i{1uz}; i <= helper->items.size(); ++i) {
            separators_->print(helper->width, i);
            std::cout << helper->build_row(i, columns_ptr_->columns(), color_conditions_);
        }
    }


    Menu_Items_t menu_items_;
    Separator* separators_;
    Columns<columns>* columns_ptr_;
    Title_t* title_ptr_;
    std::size_t* response_ptr_;
    Color_Conditions_t color_conditions_;
};

/**
 * @brief Helper class which privately inherits the Menu class.
 * Does the calculations for the dimensions of the menu for printing
 * @tparam columns number of columns
 * Represents the two menu types
 */
template <std::size_t columns>
class Menu<columns>::print_helper {

public:

    print_helper() : width(), num_of_cols() {}
    /**
     * @note Constructor
     * @memberof print_helper
     * @param menu pointer to Menu class
     * @brief calculates all necessary values for printing a menu
     */
    explicit print_helper(const Menu* menu) {

        items = menu->get_menu_items();
        num_of_cols = items.at(0).size();

        // adding numbered index to menu items in first column, format: #) menu_item[0]
        for (auto i{0uz}; i < items.size(); ++i) {
            items.at(i).at(0).insert(0, std::to_string(i + 1) + ") ");
        }

        for (auto i{0uz}; i < num_of_cols; ++i) {

            auto test_column = get_col(items, i);

            // if there are headers then they need to be included in buffer calculation
            test_column.emplace_back(menu->get_header(i + 1));

            col_dimensions_.emplace_back(max_length_in_items(test_column) + 8, 0);
        }

        width = this->get_buffer_size(menu->get_title());
    }


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


    /**
* @overload std::string build_row(const std::size_t row_index) const
* @memberof print_helper<std::vector<std::vector<std::string>>>
 */
    [[nodiscard]] std::string build_row(
        const std::size_t row_index,
        const std::array<Column_t, columns> col_chrs_ptr,
        const Color_Conditions_t &conditions) const
    {

        std::string row_str;
        std::string item{};
        auto pad = std::size_t{0uz};
        char preceding_pad_char, follow_pad_char;

        for (auto i{0uz}; i < num_of_cols; ++i) {

            pad = col_dimensions_.at(i).buffer - items[row_index - 1][i].length();

            const Align alignment = in_list(row_index, Menu_Characteristics::row::exclude_from_alignment)
                                  ? Align::LEFT : col_chrs_ptr[i].characteristics.alignment_;


            // Index must be separated from the item string for first column
            if (i == FIRST_COLUMN) {
                const auto index_pos = std::size_t{items[row_index - 1][i].find(')')};
                item = items[row_index - 1][i].substr(index_pos + 2);
                row_str.append(items[row_index - 1][i].substr(0, index_pos + 2));
            }
            else {
                item = items[row_index - 1][i];
            }


            if (const auto color_condition = conditions.find_by_col_index_to_color(i + 1); color_condition != nullptr) {
                if (items[row_index - 1][color_condition->col_index_for_condition_ - 1] == color_condition->equal_condition_) {
                    item = color_text(color_condition->color_) + item + reset_ansi;
                }
                else
                    item = color_text(color_condition->else_color_) + item + reset_ansi;
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
                    row_str.append(item + std::string(pad, preceding_pad_char));
                    break;
                case Align::RIGHT:
                    if (col_chrs_ptr[i].preceding_dots && !in_list(row_index, Menu_Characteristics::row::exclude_from_dots))
                        follow_pad_char = '.';
                    else
                        follow_pad_char = ' ';
                    row_str.append(std::string(pad, follow_pad_char) + item);
                    break;
                case Align::CENTER:

                    const auto half_pad = pad / 2;
                    const auto remainder = pad % 2;

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
        row_str.append(reset_ansi_nl);
        return row_str;
    }


    /** @memberof print_helper. Stores the menu items */
    std::vector<std::vector<std::string>> items;

    std::vector<col_dimensions> col_dimensions_;

    /** @memberof print_helper. Total width of the menu */
    std::size_t width;

    /** @memberof print_helper. Total number of columns */
    std::size_t num_of_cols;
};



#endif // MENU_LIBRARY_H