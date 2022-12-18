#pragma once
#include <concepts>
#include <optional>
#include <string_view>
#include <string>
#include "utils.h"

namespace cfmt 
{
    //format_spec ::= [[fill]align][sign]["#"]["0"][width]["." precision]["L"][type]
    struct FormatDescriptor {
        std::optional<std::pair<char, char>> fill_align;
        char sign = ' ';
        bool numb_prefix = false;
        bool zero_pad = false;
        uint32_t width = 0;
        uint32_t precision = 0;
        bool localized = false;
        std::optional<char> type;

        template <class It>
        constexpr auto parse_fill_align(It begin, It end) noexcept {
            if (begin == end) {
                return begin;
            }
            if (*begin == '<' || *begin == '>' || *begin == '^') {
                fill_align = std::make_pair(' ', *begin);
                return std::next(begin);
            }
            char fill = *begin;
            auto it = std::next(begin);
            if (it == end) {
                return begin;
            }
            if (*it == '<' || *it == '>' || *it == '^') {
                fill_align = std::make_pair(fill, *it);
                return std::next(it);
            }
            return begin;
        }
        template <class It>
        constexpr auto parse_sign(It begin, It end) noexcept {
            if (begin == end) {
                return begin;
            }
            switch (*begin) {
                case '+':
                case '-':
                case ' ':
                    sign = *begin;
                    return ++begin;
            }
            return begin;
        }
        template <class It>
        constexpr auto parse_numb_prefix(It begin, It end) noexcept {
            if (begin == end) {
                return begin;
            }
            if (*begin == '#') {
                numb_prefix = true;
                return ++begin;
            }
            return begin;
        }
        template <class It>
        constexpr auto parse_zero_pad(It begin, It end) noexcept {
            if (begin == end) {
                return begin;
            }
            if (*begin == '0') {
                zero_pad = true;
                return ++begin;
            }
            return begin;
        }
        template <class It>
        constexpr auto parse_width(It begin, It end) noexcept {
            if (begin == end) {
                return begin;
            }
            while (begin != end && *begin >= '0' && *begin <= '9') {
                width = width*10 + (*begin - '0');
                ++begin;
            }
            return begin;
        }
        template <class It>
        constexpr auto parse_precision(It begin, It end) noexcept {
            /// TODO: implement float precision
            // if (begin == end) {
            //     return begin;
            // }
            // if (*begin == '.') {
            //     ++begin;
            //     while (begin != end && *begin >= '0' && *begin <= '9') {
            //         precision = precision*10 + (*begin - '0');
            //         ++begin;
            //     }
            // }
            return begin;
        }
        template <class It>
        constexpr auto parse_localized(It begin, It end) noexcept {
            if (begin == end) {
                return begin;
            }
            if (*begin == 'L') {
                localized = true;
                return ++begin;
            }
            return begin;
        }
        template <class It>
        constexpr auto parse_type(It begin, It end) noexcept {
            if (begin == end) {
                return begin;
            }
            switch (*begin) {
                case 'b': // binary lower
                case 'B': // binary upper
                case 'c': // char
                case 'd': // decimal
                case 'o': // octal
                case 'x': // hex lower
                case 'X': // hex upper
                // case 'a':
                // case 'A':
                // case 'e':
                // case 'E':
                // case 'f':
                // case 'F':
                // case 'g':
                // case 'G':
                case 'p': // pointer
                case 's': // string
                    type = *begin;
                    return ++begin;
            }
            return begin;
        }
        template <class It>
        constexpr auto parse(It begin, It end) noexcept {
            begin = parse_fill_align(begin, end);
            begin = parse_sign(begin, end);
            begin = parse_numb_prefix(begin, end);
            begin = parse_zero_pad(begin, end);
            begin = parse_width(begin, end);
            begin = parse_precision(begin, end);
            begin = parse_localized(begin, end);
            begin = parse_type(begin, end);
            return begin;
        }
    };
    template <class T>
    struct Formatter;

    template <class StringT>
        requires std::same_as<std::string_view, StringT> || std::convertible_to<StringT, std::string_view>
    struct Formatter<StringT> {
        constexpr auto parse(std::string_view input) const -> FormatDescriptor {
            FormatDescriptor desc;
            desc.parse(input.begin(), input.end());
            return desc;
        }
        template <class DescriptorType>
        constexpr auto format(const StringT& input, const DescriptorType& desc) const -> std::string{
            auto str = std::string_view(input);
            std::string result;
            result.reserve(std::max<size_t>(str.length(),desc.width));
            if (str.length() >= desc.width) {
                result.append(str);
            } else {
                auto [fill, align] = desc.fill_align.value_or(std::make_pair(' ', '<'));
                if (align == '<') {
                    result.append(str);
                    result.append(desc.width-str.length(), fill);
                } else if (align == '>') {
                    result.append(desc.width-str.length(), fill);
                    result.append(str);
                } else if (align == '^') {
                    auto left = std::max<int32_t>(0,desc.width-str.length())/2;
                    auto right = desc.width-str.length()-left;
                    result.append(left, fill);
                    result.append(str);
                    result.append(right, fill);
                }
            }
            return result;
        }
    };

    template <std::integral IntegralT>
    struct Formatter<IntegralT> : Formatter<std::string_view>
    {
        template <class DescriptorType>
        constexpr auto format(IntegralT input, const DescriptorType& desc) const -> std::string {
            return Formatter<std::string_view>::format(int_to_string(input, desc), desc);
        }
        template <class DescriptorType>
        constexpr auto int_to_string(std::integral auto input, const DescriptorType& params) const -> std::string {
            constexpr char digits[] = "0123456789abcdef";
            std::string result;
            result.reserve(sizeof(IntegralT)*8 + 2 + (params.sign ? 1 : 0));
            auto base = 10;
            auto is_upper = false;
            switch (params.type.value_or('d')) {
                case 'b': base = 2; break;
                case 'B': base = 2; is_upper = true; break;
                case 'o': base = 8; break;
                case 'x': base = 16; break;
                case 'X': base = 16; is_upper = true; break;
                case 'd': base = 10; break;
                default: break;
            }
            auto to_upper = is_upper 
                ? [](char ch) { return utils::to_upper(ch); } 
                : [](char ch) { return ch; };
            if constexpr (std::signed_integral<decltype(input)>) {
                if (input < 0) {
                    result.push_back('-');
                    input = -input;
                }
            }
            if (params.numb_prefix) {
                if (base == 16) {
                    result.push_back('0');
                    result.push_back(to_upper('x'));
                } else if (base == 8) {
                    result.push_back('0');
                } else if (base == 2) {
                    result.push_back('0');
                    result.push_back(to_upper('b'));
                }
            }
            if (input == 0) {
                result.push_back('0');
            } else {
                auto begin = result.end();
                while (input > 0) {
                    result.push_back(to_upper(digits[input % base]));
                    input /= base;
                }
                std::reverse(begin, result.end());
            }
            return result;
        }
    };
}
