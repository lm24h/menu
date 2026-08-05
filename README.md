# Menu

A C++ header-only library for generating formatted console menus with response handling

## Requirements

- C++23
- CMake 4.1+

## Installation

### Option 1: Add as a subdirectory

Clone (or add as a Git submodule) into your project:

```
MyProject/
├── CMakeLists.txt
├── src/
└── external/
    └── menu/
```

Then in your `CMakeLists.txt`:

```cmake
add_subdirectory(external/menu)

target_link_libraries(MyProgram PRIVATE menu)
```

### Option 2: FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
    menu
    GIT_REPOSITORY https://github.com/lm24h/menu.git
    GIT_TAG v1.0.0
)

FetchContent_MakeAvailable(menu)

target_link_libraries(MyProgram PRIVATE menu)
```

## Usage

Include the library:

```cpp
#include <menu/menu.h>
```

Create a menu from a 2D vector of strings:

```cpp
std::vector<std::vector<std::string>> items{
    {"Burger", "$8.99"},
    {"Pizza", "$11.50"},
    {"Salad", "$6.25"}
};

Menu menu(items);
```

Add optional formatting:

```cpp
menu.title("Restaurant Menu");

menu.headers(
    "Item",
    "Price"
);

menu.separators(
    '-', 2,
    '=', 3
);
```

Print the menu:

```cpp
menu.print();
```

Set Response:

```cpp
menu.response(2)
```

Get Response:

```cpp
menu.response()
```

## Example

```cpp
#include <iostream>
#include <menu/menu.h>

int main() {
    
    std::vector<std::vector<std::string>> items{
        {"Burger", "$8.99"},
        {"Pizza", "$11.50"},
        {"Salad", "$6.25"}
    };

    Menu menu(items);

    menu.title("Restaurant Menu");

    menu.headers(
        "Item",
        "Price"
    );

    menu.separators(
        '-', 2,
        '=', 3
    );

    menu.print();
    
    menu.response(2);
    
    std::cout << "Integer Response: " << menu.response() << "\n";

}
```

Output:

```
    Restaurant Menu
~~~~~~~~~~~~~~~~~~~~~~~
Item             Price
-----------------------
1) Burger        $8.99
-----------------------
2) Pizza         $11.50
=======================
3) Salad         $6.25
~~~~~~~~~~~~~~~~~~~~~~~
Integer Response: 2
```

## Features

- Automatic column sizing and formatting
- Optional title
- Optional column headers
- Configurable separator lines
- Supports one- and two-dimensional string containers
- Response handling 
- Modern C++23 API

## Exceptions

Some functions validate their arguments and throw 
`std::invalid_argument` if invalid input is provided
or `std::runtime_error` if response is used when not set.


For example:
- `headers()` throws if the number of headers does not match the number of columns.
- `separators()` throws if an invalid column index is specified.
- `response()` throws if response has not been set

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.