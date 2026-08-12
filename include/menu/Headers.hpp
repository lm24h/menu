#pragma once
#include <array>
#include <stdexcept>
#include "menu_types.hpp"
#include "Col_Characteristics.hpp"

template <std::size_t size>
class Headers {
public:
    Headers() {
        clear();
    }

    constexpr void set_headers(std::ranges::input_range auto&& headers) {
        if (std::ranges::size(headers) != size)
            throw std::invalid_argument("\nHeaders count mismatch");
        for (std::size_t i = 0; i < size; ++i)
            headers_[i] = headers[i];
    }

    constexpr void set_headers(const std::initializer_list<std::string>& headers) {
        if (headers.size() > size)
            throw std::out_of_range(
                "\nNumber of header arguments ->(" + std::to_string(headers.size()) +
                ") > number of cols ->(" + std::to_string(size) + ")");
        auto i{0uz};
        for (const auto& header : headers)
            headers_[i++] = header;
    }

    std::string get_header(const std::size_t index) {
        if (index > size || index == 0)
            throw std::out_of_range(
                "Entered col index ->(" + std::to_string(index) +
                ") outside exceptable col range ->[1, " + std::to_string(size) + "]"
            );
        return headers_[index - 1];
    }

    constexpr bool empty() noexcept {
        for (const auto& header : headers_)
            if (header != "")
                return false;
        return true;
    }

    constexpr void clear() noexcept {
        for (auto& header : headers_)
            header = "";
    }


private:
    std::array<std::string, size> headers_;
    Col_Characteristics characteristics_;
};