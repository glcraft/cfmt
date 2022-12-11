#pragma once
#include <concepts>
#include <string_view>
#include <string>
#include "utils.h"

template <class T>
struct Formatter;

template <class StringT>
    requires std::same_as<std::string_view, StringT> || std::convertible_to<StringT, std::string_view>
struct Formatter<StringT> {
    int16_t fill = 0;
    char align = '<';
    char sign = ' ';
    uint32_t width = 0;
    constexpr Formatter() = default;
    constexpr Formatter(std::string_view format_text) {
        std::optional<char> align;
        for (auto ch : format_text) {
            if (ch >= '0' && ch <= '9') {
                fill = fill*10 + (ch-'0');
            } else if (ch == '<' || ch == '>' || ch == '^') {
                this->align = ch;
            } else if (ch == '+' || ch == '-' || ch == ' ') {
                sign = ch;
            }
        }
    }

    constexpr auto format(const StringT& input) const -> std::string{
        auto str = std::string_view(input);
        std::string result;
        result.reserve(std::max<size_t>(str.length(),fill));
        if (align == '<') {
            result.append(str);
            if (str.length() < fill) {
                result.append(fill-str.length(), sign);
            }
        } else if (align == '>') {
            if (str.length() < fill) {
                result.append(fill-str.length(), sign);
            }
            result.append(str);
        } else if (align == '^') {
            if (str.length() >= fill) {
                result.append(str);
            } else {
                auto left = std::max<int32_t>(0,fill-str.length())/2;
                auto right = fill-str.length()-left;
                result.append(left, sign);
                result.append(str);
                result.append(right, sign);
            }
        }
        return result;
    }
};

template <std::integral IntegralT>
struct Formatter<IntegralT> : Formatter<std::string_view>
{
    constexpr Formatter() = default;
    constexpr Formatter(std::string_view format_text) : Formatter<std::string_view>(format_text) 
    {}
    constexpr auto format(IntegralT input) const -> std::string {
        return Formatter<std::string_view>::format(cfmt::utils::int_to_string(input));
    }
};