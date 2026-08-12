#pragma once
#include <cstring>
#include "menu_types.hpp"


struct Title_t {
    ~Title_t() {
        std::free(title_);
    }


    Title_t(const Title_t& other) {
        length_ = other.length_;
        title_alignment_ = other.title_alignment_;
        title_color_ = other.title_color_;
        title_style_ = other.title_style_;
        title_ = static_cast<char*>(std::malloc(length_ + 1));
        for (auto i{0uz}; i < length_; ++i)
            title_[i] = other.title_[i];
        title_[length_] = '\0';
    }


    Title_t(Title_t&& other) noexcept :
        title_{other.title_},
        length_{other.length_},
        title_alignment_{other.title_alignment_},
        title_color_{other.title_color_},
        title_style_{other.title_style_}
    {
        other.length_ = 0;
        other.title_ = nullptr;
        other.title_alignment_ = Align::CENTER;
        other.title_color_ = Color::WHITE;
        other.title_style_ = Style::NONE;
    }


    explicit Title_t(
        const char* const title,
        const Align alignment=Align::CENTER,
        const Color color=Color::WHITE ,
        const Style style=Style::NONE) :

        title_{static_cast<char*>(std::malloc(std::strlen(title) + 1))},
        length_{static_cast<unsigned int>(std::strlen(title))},
        title_alignment_{alignment},
        title_color_{color},
        title_style_{style}
    {
        for (auto i{0uz}; i < length_; ++i)
            title_[i] = title[i];
        title_[length_] = '\0';
    }


    Title_t& operator=(const Title_t& other) {
        if (this == &other)
            return *this;

        const auto new_title = static_cast<char*>(std::malloc(other.length_ + 1));

        for (auto i{0uz}; i < other.length_; ++i)
            new_title[i] = other.title_[i];
        new_title[other.length_] = '\0';

        std::free(title_);
        title_ = new_title;
        length_ = other.length_;
        title_alignment_ = other.title_alignment_;
        title_color_ = other.title_color_;
        title_style_ = other.title_style_;

        return *this;

    }


    Title_t& operator=(Title_t&& other) noexcept {
        if (this == &other)
            return *this;

        std::free(title_);

        title_ = other.title_;
        length_ = other.length_;
        title_alignment_ = other.title_alignment_;
        title_color_ = other.title_color_;
        title_style_ = other.title_style_;

        other.title_ = nullptr;
        other.length_ = 0;
        other.title_alignment_ = Align::CENTER;
        other.title_color_ = Color::WHITE;
        other.title_style_ = Style::NONE;
        return *this;
    }


    constexpr char* c_str() const noexcept {
        return title_;
    }


    constexpr void set_align(const Align alignment) noexcept { title_alignment_ = alignment; }
    constexpr void set_color(const Color& color) noexcept { title_color_ = color; }
    constexpr void set_style(const Style style) noexcept { title_style_ = style; }
    constexpr Align get_align() const noexcept { return title_alignment_; }
    constexpr Color get_color() const noexcept { return title_color_; }
    constexpr Style get_style() const noexcept { return title_style_; }

private:
    char* title_;
    unsigned int length_;
    Align title_alignment_;
    Color title_color_;
    Style title_style_;
};
