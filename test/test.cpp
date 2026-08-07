//
// Created by ljm03 on 7/28/2026.
//
#include "../include/menu/menu.h"
#include <vector>
#include <string>

#define TEST_VALUE1 5
#define TEST_VALUE2 30

int main() {

    std::vector<std::vector<std::string>> items{
        {"Burger", "$8.99"},
        {"Pizza", "$11.50"},
        {"Salad", "$6.25"},
        {"Exit", ""}
    };

    Menu menu(items);

    menu.title("Restaurant Menu");

    menu.headers({"Item", "Price"});

    menu.separators(
        '=', 4
    );

    menu.align(2, Align::RIGHT);
    menu.align_header(2, Align::RIGHT);

    menu.print();

    menu.response(2);

    std::cout << "Integer Response: " << menu.response() << "\n";
}