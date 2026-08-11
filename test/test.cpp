//
// Created by ljm03 on 7/28/2026.
//
#include "../include/menu/menu.h"
#include <vector>
#include <string>

#define TESTVALUE 31
#define RESETTESTVALUE 50

namespace tests_2d {
    void main();
    void title();
}

namespace tests_1d {
    void main();
}

int main() {


    //tests_2d::main();
    tests_2d::title();


}

void tests_2d::main() {

    std::vector<std::vector<std::string>> items{
            {"Burger", "Order-in", "$8.99"},
            {"Pizza", "Order-out", "$11.50"},
            {"Salad", "Order-in", "$6.25"},
        };

    Menu<str_vec_2d_t> menu(items);
    menu.emplace_back({"EXIT", "", ""});

    menu.title("Restaurant Menu", Align::CENTER);

    menu.add_headers("Item", "In-or-Out", "Price");

    menu.separators('=', 4);

    try {
        menu.align(3, Align::RIGHT, 4, Align::RIGHT);
    }
    catch (...) {
        std::cerr << "Exception caught!\n";
        menu.align(3, Align::RIGHT);
    }

    menu.excl_align({4});
    menu.align_header(3, Align::CENTER, 2, Align::LEFT);
    menu.preceding_dots(3, {4});

    menu.print();

    menu.response(2);

    std::cout << "Integer Response: " << menu.response() << "\n";
    std::cout << "Size of Menu: " << sizeof(menu) << " bytes\n";
}

void tests_2d::title() {
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