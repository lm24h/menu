//
// Created by ljm03 on 7/28/2026.
//
#include "../include/menu/menu.h"
#include <vector>
#include <string>
#include <chrono>

#define TESTVALUE 31
#define RESETTESTVALUE 50
#define TESTLOOP 10000

namespace tests_2d {
    int main();
    int title();
}

namespace tests_1d {
    void main();
}

void clear_prev_lines(const std::size_t num_lines) {
    for (auto i{0uz}; i < num_lines; ++i) {
        std::cout << "\033[2K";
        if (i + 1 < num_lines)
            std::cout << "\033[1A";
    }
    std::cout << "\r" << std::flush;
}

int main() {



    tests_2d::main();

    return 0;

}

int tests_2d::main() {

    std::vector<std::vector<std::string>> items{
            {"Burger", "Order-in", "$8.99"},
            {"Pizza", "Order-out", "$11.50"},
            {"Salad", "Order-in", "$6.25"},
        };

    Menu<str_vec_2d_t> menu(items);
    menu.emplace_back({"EXIT", "", ""});

    menu.title("");
    menu.add_headers("Item", "In-or-Out", "Price");
    menu.separators('=', 4);
    menu.align(3, Align::RIGHT);
    menu.excl_align({4});
    menu.align_header(3, Align::CENTER, 2, Align::LEFT);
    menu.preceding_dots(3, {4});
    menu.print();

    Menu<str_vec_2d_t> menu2(items);
    menu2 = menu;
    menu2.print();


    // std::chrono::duration<long long, std::ratio<1, 1000000>> total_elapsed{0};
    //
    // for (auto i{0uz}; i < TESTLOOP; ++i) {
    //     auto start = std::chrono::steady_clock::now();
    //     menu.print();
    //     auto end = std::chrono::steady_clock::now();
    //     total_elapsed += std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    //     clear_prev_lines(12);
    // }
    //
    // std::cout << "Time to print: " << total_elapsed.count() / TESTLOOP << " us" << std::endl;

    return 0;
}



int tests_2d::title() {
    std::vector<std::vector<std::string>> items{
                {"Burger", "Order-in", "$8.99"},
                {"Pizza", "Order-out", "$11.50"},
                {"Salad", "Order-in", "$6.25"},
            };

    Menu<str_vec_2d_t> menu(items);
    menu.emplace_back({"EXIT", "", ""});

    menu.title("Restaurant Menu", Align::CENTER);
    menu.title("Restaurant Menu2", Align::RIGHT);

    menu.separators('=', 4);
    menu.excl_align({4});

    menu.print();

    menu.reset();

    return 0;
}


void tests_1d::main() {
    str_vec_1d_t items_1d = {};
    for (auto i{0uz}; i < TESTVALUE; ++i) {
        if (i==1)
            items_1d.emplace_back("Burger");
        else
            items_1d.emplace_back("Test");
    }
    Menu<str_vec_1d_t> menu_1d(items_1d);
    menu_1d.title("1D Menu Test");
    menu_1d.print();

    menu_1d.response(2);

    std::cout << "Integer Response: " << menu_1d.response() << "\n";
    std::cout << "Response as item: " << menu_1d.response_as_menu_item() << "\n";
    std::cout << "Size of Menu in bytes: " << sizeof(menu_1d) << "\n";
    std::cout << "Size of Menu in items: " << menu_1d.size() << "\n";


    str_vec_1d_t reset_items_1d = {};
    for (auto i{0uz}; i < RESETTESTVALUE; ++i) {
        if (i==1)
            reset_items_1d.emplace_back("Burger");
        else
            reset_items_1d.emplace_back("Test");
    }
    menu_1d.reset();
    menu_1d.menu_items(reset_items_1d);
    menu_1d.title("1D Menu Test");
    menu_1d.print();

    menu_1d.response(2);
    reset_items_1d = menu_1d.menu_items();

    std::cout << "Integer Response: " << menu_1d.response() << "\n";
    std::cout << "Response as item: " << menu_1d.response_as_menu_item() << "\n";
    std::cout << "Size of Menu in bytes: " << sizeof(menu_1d) << "\n";
    std::cout << "Size of Menu in items: " << menu_1d.size() << "\n";
}