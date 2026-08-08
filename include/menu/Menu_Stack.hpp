//
// Created by ljm03 on 8/8/2026.
//

#ifndef MENU_MENU_STACK_H
#define MENU_MENU_STACK_H

#include <string>
#include <vector>
#include <memory>

template <typename T>
struct Menu_Item_Stack {
    Menu_Item_Stack() : Sptr(nullptr), Bptr(nullptr) {}
    ~Menu_Item_Stack() {
        free_stack();
    }

    constexpr void init(const std::size_t size) noexcept {
        Bptr = static_cast<T*>(std::malloc(sizeof(T) * size));
        Sptr = Bptr;
    }

    constexpr void stack(T item) {
        if (Bptr == nullptr)
            throw std::runtime_error("Stack not initialized");

        const std::size_t old_size = size();
        T* new_ptr = static_cast<T*>( std::malloc(sizeof(T) * (old_size + 1)) );

        if (new_ptr == nullptr)
            throw std::bad_alloc();

        // Move existing
        for (auto i{0}; i < old_size; ++i)
            std::construct_at(new_ptr + i, std::move(Bptr[i]));

        // construct new
        std::construct_at(new_ptr + old_size, std::move(item));

        // destroy old
        for (auto i{0uz}; i < old_size; ++i)
            std::destroy_at(Bptr + i);

        std::free(Bptr);

        Bptr = new_ptr;
        Sptr = Bptr + old_size + 1;
    }

    [[nodiscard]] constexpr T* at(const std::size_t index) const noexcept {
        if (!empty() && index < size())
            return Bptr + index;
        return nullptr;
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept {
        if (Bptr == nullptr)
            return 0;
        return static_cast<std::size_t>(Sptr - Bptr);
    }

    [[nodiscard]] constexpr bool empty() const noexcept { return Bptr == nullptr || Bptr == Sptr; }

    [[nodiscard]] constexpr T* base_addr() const noexcept { return Bptr; }

    constexpr void clear_menu_items() {
        while (Sptr != Bptr) {
            --Sptr;
            std::destroy_at(Sptr);
        }
    }

    constexpr void free_stack() noexcept {
        clear_menu_items();
        std::free(Bptr);
        Bptr = nullptr;
        Sptr = nullptr;
    }

private:
    T* Sptr;
    T* Bptr;
};

#endif //MENU_MENU_STACK_H