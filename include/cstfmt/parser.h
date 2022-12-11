#pragma once
#include <cstddef>
#include <string_view>
#include <string>
#include <optional>
#include <concepts>
#include "strlit.h"
#include "utils.h"

template <class T>
struct ArgumentFormat;

template <class StringT>
    requires std::same_as<std::string_view, StringT> || std::convertible_to<StringT, std::string_view>
struct ArgumentFormat<StringT> {
    int16_t fill = 0;
    char align = '<';
    char sign = ' ';
    uint32_t width = 0;
    constexpr ArgumentFormat() = default;
    constexpr ArgumentFormat(uint32_t fill, char align, char sign) : fill(fill), align(align), sign(sign) 
    {}
    constexpr ArgumentFormat(std::string_view format_text) {
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
struct ArgumentFormat<IntegralT> : ArgumentFormat<std::string_view>
{
    constexpr ArgumentFormat() = default;
    constexpr ArgumentFormat(uint32_t fill, char align, char sign) : ArgumentFormat<std::string_view>(fill, align, sign)
    {}
    constexpr ArgumentFormat(std::string_view format_text) : ArgumentFormat<std::string_view>(format_text) 
    {}
    constexpr auto format(IntegralT input) const -> std::string {
        return ArgumentFormat<std::string_view>::format(cfmt::utils::int_to_string(input));
    }
};

struct Token {
    int64_t id=-1;
    std::string format;
    constexpr Token(int64_t id, std::string_view format) : id(id), format(format) {}
    constexpr Token(std::string_view format) : format(format) {}
    constexpr Token(std::string&& format) : format(std::move(format)) {}
    constexpr auto  is_arg() const {
        return id != -1;
    }
};
constexpr auto parse(std::string_view text) {
    std::vector<Token> result;
    int32_t level=0;
    size_t prev_pos = 0;
    uint32_t current_index = 0;
    auto get_index = [&](std::optional<std::string_view> str) {
        if (str && !str->empty()) {
            return cfmt::utils::sv_to_int(*str);
        }
        return static_cast<int64_t>(current_index++);
    };
    auto add_string_to_last = [&](auto... to_add) {
        if (result.empty()) {
            result.emplace_back(std::string(to_add...));
        }
        auto& last = result.back();
        last.format.append(to_add...);
    };
    for(size_t pos = 0;pos<text.length();pos++) {
        auto ch = text[pos];
        if (ch == '{') {
            if (level == 0) {
                if (pos < text.length()-1 && text[pos+1] == '{') {
                    add_string_to_last('{', 1);
                    continue;
                }
                result.emplace_back(std::move(text.substr(prev_pos, pos-prev_pos)));
            }
            level++;
            prev_pos = pos+1;
        } else if (ch == '}') {
            if (level == 0 && pos < text.length()-1 && text[pos+1] != '}') {
                add_string_to_last('}', 1);
                continue;
            }
            level--;
            if (level == 0) {
                auto arg = text.substr(prev_pos, pos-prev_pos);
                auto pos = arg.find(':');
                if (pos == std::string::npos) {
                    result.push_back(Token{get_index(std::nullopt), std::string()});
                } else {
                    result.push_back(Token{get_index(arg.substr(0, pos)), std::string(arg.substr(pos+1))});
                }
            } else if (level < 0) {
                // throw std::runtime_error("Unmatched '}'");
            }
            prev_pos = pos+1;
        }
    }
    if (prev_pos != text.length()) {
        result.push_back(std::string(text.substr(prev_pos)));
    }
    return result;
}
constexpr auto format_string(std::string_view format_text, auto... args) -> std::string; 

template <size_t N = 10000>
constexpr auto format(std::string_view format_text, auto... args) -> strlit::String<N>
{
    strlit::String<N> res;
    auto formatted = format_string(format_text, args...);
    std::copy(formatted.begin(), formatted.end(), res.text);
    std::fill(res.text+formatted.length(), res.text+res.size, '\0');
    return res;
}

constexpr auto format_string(std::string_view format_text, auto... args) -> std::string {
    auto parsed = parse(format_text);
    std::string result;
    for (auto& arg : parsed) {
        if (!arg.is_arg()) {
            result.append(arg.format);
        } else {
            // if (arg.id >= sizeof...(args)) {
                // throw std::runtime_error("Not enough arguments");
            // }
            arg.format = format(std::string_view{arg.format}, args...);
            int i=0;
            ([&result, &arg, &i](auto&& arg_value) mutable {
                if (i++ == arg.id) {
                    result.append(ArgumentFormat<std::remove_cvref_t<decltype(arg_value)>>{arg.format}.format(arg_value));
                    return true;
                }
                return false;
            }(args) || ...);
        }
    }
    return result;
}

static constexpr size_t format_length(std::string_view format_text, auto... args) 
{
    return format(format_text, args...).length();
}