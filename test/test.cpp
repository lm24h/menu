//
// Created by ljm03 on 7/28/2026.
//
#include "../include/menu/menu.h"
#include <vector>
#include <string>

#define TESTVALUE 31
#define RESETTESTVALUE 50

int main() {

    std::vector<std::vector<std::string>> items{
        {"Burger", "$8.99"},
        {"Pizza", "$11.50"},
        {"Salad", "$6.25"},
    };

    Menu<str_vec_2d_t> menu(items);
    menu.emplace_back({"EXIT", ""});

    menu.title("Restaurant Menu");

    menu.headers({"Item", "Price"});

    menu.separators(
        '=', 4
    );

    menu.align(1, Align::RIGHT);
    menu.align_header(1, Align::RIGHT);

    menu.print();

    menu.response(2);

    std::cout << "Integer Response: " << menu.response() << "\n";
    std::cout << "Size of Menu: " << sizeof(menu) << " bytes\n";




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