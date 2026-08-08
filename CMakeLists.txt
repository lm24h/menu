cmake_minimum_required(VERSION 4.1)

project(
        menu
        VERSION 1.0.0
        LANGUAGES CXX
)

add_library(menu INTERFACE
        include/menu/Menu_Stack.hpp)

# Make header visible to anything linking the library
target_include_directories(menu INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(menu INTERFACE
        cxx_std_23
)

add_executable(menu_tests
        test/test.cpp
        include/menu/Menu_Stack.hpp
)
target_link_libraries(menu_tests PRIVATE
        menu
)

option(MENU_BUILD_TESTS "Build menu tests" OFF)

if (MENU_BUILD_TESTS)
    add_executable(menu_tests
        test/test.cpp
    )
    target_link_libraries(menu_tests PRIVATE
            menu
    )
endif()

install(
        TARGETS menu
        EXPORT menuTargets
)

install(
        DIRECTORY include/
        DESTINATION include
)