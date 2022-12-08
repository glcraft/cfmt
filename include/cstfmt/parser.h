#include <_types/_uint8_t.h>
#include <cstddef>
#include <string_view>
#include <string>
#include <optional>
#include "strlit.h"

struct Token;
struct PlainText  {
    std::string text;
};
struct Argument {
    uint8_t index = -1;
    std::optional<std::vector<Token>> format;
};
struct Token {
    enum class Type {
        PlainText,
        Argument
    } type;
    std::optional<PlainText> plain_text;
    std::optional<Argument> argument;
    constexpr Token(PlainText&& text) : plain_text(text), argument(std::nullopt), type(Type::PlainText) {}
    constexpr Token(Argument&& arg) : plain_text(std::nullopt), argument(arg), type(Type::Argument) {}
    constexpr Token(const Token& other) : plain_text(other.plain_text), argument(other.argument), type(other.type) {}
    constexpr Token(Token&& other) : plain_text(std::move(other.plain_text)), argument(std::move(other.argument)), type(other.type) {}
    constexpr ~Token()  = default;
};

struct ArgumentFormat {
    uint32_t fill = 0;
    char align = '<';
    char sign = ' ';
    constexpr ArgumentFormat() = default;
    constexpr ArgumentFormat(uint32_t fill, char align, char sign) : fill(fill), align(align), sign(sign) 
    {}
    constexpr ArgumentFormat(std::string_view format_text) {
        std::optional<char> align;
        for (auto ch : format_text) {
            if (ch >= '0' && ch <= '9') {
                fill = fill*10 + (ch-'0');
            } else if (ch == '<' || ch == '>' || ch == '^') {
                align = ch;
            } else if (ch == '+' || ch == '-' || ch == ' ') {
                sign = ch;
            }
        }
    }

    constexpr std::string format(std::string_view str) const {
        std::string result;
        result.reserve(std::max<size_t>(str.length(),fill));
        if (align == '<') {
            result.append(str);
            result.append(fill-str.length(), sign);
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

constexpr uint32_t to_int(std::string_view str) {
    size_t result = 0;
    for (auto c : str) {
        result = result*10 + (c-'0');
    }
    return result;
}
struct Arg {
    uint32_t id;
    std::string format;
};
constexpr auto parse(std::string_view text) {
    std::vector<std::variant<std::string, Arg>> result;
    int32_t level=0;
    size_t prev_pos = 0;
    uint32_t current_index = 0;
    auto get_index = [&](std::optional<std::string_view> str) {
        if (str) {
            return to_int(*str);
        } else {
            return current_index++;
        }
    };
    auto add_string_to_last = [&](auto... to_add) {
        if (result.empty()) {
            result.push_back(std::string(to_add...));
        }
        auto& last = result.back();
        if (std::holds_alternative<std::string>(last)) {
            auto& str = std::get<std::string>(last);
            str.append(to_add...);
        } else {
            auto& arg = std::get<Arg>(last);
            arg.format.append(to_add...);
        }
    };
    for(size_t pos = 0;pos<text.length();pos++) {
        auto ch = text[pos];
        if (ch == '{') {
            if (level == 0) {
                if (pos < text.length()-1 && text[pos+1] == '{') {
                    add_string_to_last('{', 1);
                    continue;
                }
                result.push_back(std::string(text.substr(prev_pos, pos-prev_pos)));
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
                    result.push_back(Arg{get_index(std::nullopt), ""});
                } else {
                    result.push_back(Arg{to_int(arg.substr(0, pos)), std::string(arg.substr(pos+1))});
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

constexpr auto format(std::string_view format_text, auto... args) -> std::string {
    auto parsed = parse(format_text);
    std::string result;
    for (auto& item : parsed) {
        if (std::holds_alternative<std::string>(item)) {
            result.append(std::get<std::string>(item));
        } else {
            auto& arg = std::get<Arg>(item);
            // if (arg.id >= sizeof...(args)) {
                // throw std::runtime_error("Not enough arguments");
            // }
            arg.format = format(std::string_view{arg.format}, args...);
            ([&result, &arg, i=0](auto&& arg_value) mutable {
                if (i++ == arg.id) {
                    result.append(ArgumentFormat{arg.format}.format(arg_value));
                    return true;
                }
                return false;
            }(args) || ...);
            
        }
    }
    return result;
}

template <strlit::StringType to_parse, strlit::StringType... Args>
struct Format : strlit::details::BaseString<format(to_parse, Args...).length()> {
    constexpr Format() : strlit::details::BaseString<format(to_parse, Args...).length()>() {
        auto result = format(to_parse, Args...);
        std::copy(result.begin(), result.end(), this->text);
    }
};