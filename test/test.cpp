//
// Created by ljm03 on 7/28/2026.
//
#include "../include/menu/menu.h"
#include <vector>
#include <string>


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

}