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

struct ArcumentFormat {
    uint32_t fill = 0;
    char align = '<';
    char sign = ' ';
    std::string format(std::string_view str) const {
        std::string result;
        result.reserve(str.length()+fill);
        if (align == '<') {
            result.append(str);
            result.append(fill-str.length(), ' ');
        } else if (align == '>') {
            result.append(fill-str.length(), ' ');
            result.append(str);
        } else if (align == '^') {
            auto left = (fill-str.length())/2;
            auto right = fill-str.length()-left;
            result.append(left, ' ');
            result.append(str);
            result.append(right, ' ');
        }
        return result;
    }
};

struct Parser {
    struct Character {
        char c;
        size_t pos;
        bool escaped = false;
    };
    constexpr Parser(std::string_view str, size_t pos = 0) : text(str) 
    {}
    constexpr size_t to_int(std::string_view str) const {
        size_t result = 0;
        for (auto c : str) {
            result = result*10 + (c-'0');
        }
        return result;
    }

    constexpr auto build_tree() const {
        std::vector<Token> result;
        size_t current_index = 0;
        std::vector<Argument> stack_arguments;
        auto prev_pos = 0;
        auto add_string_to_result = [&](auto&& str) {
            if (!str.empty()) {
                if (stack_arguments.empty()) {
                    result.push_back(PlainText{std::string(str)});
                } else {
                    if (stack_arguments.back().format == std::nullopt)
                        stack_arguments.back().format.emplace();
                    stack_arguments.back().format->emplace_back(PlainText{std::string(str)});
                }
            }
        };
        auto get_index = [&](auto pos) {
            if (prev_pos != pos) {
                return to_int(text.substr(prev_pos, pos-prev_pos));
            } else {
                return current_index++;
            }
        };
        for (auto pos = 0;pos<text.length();pos++) {
            auto ch = text[pos];
            if (ch == '{') {
                add_string_to_result(text.substr(prev_pos, pos-prev_pos));
                stack_arguments.push_back(Argument{});
                prev_pos = pos+1;
            } else if (ch == '}') {
                // if (stack_arguments.empty()) {
                //     throw std::runtime_error("Unexpected '}'");
                // }
                if (stack_arguments.back().index == static_cast<uint8_t>(-1)) {
                    stack_arguments.back().index = get_index(pos);
                } else {
                    add_string_to_result(text.substr(prev_pos, pos-prev_pos));
                }
                auto arg = std::move(stack_arguments.back());
                stack_arguments.pop_back();
                if (stack_arguments.empty()) {
                    result.push_back(std::move(arg));
                } else {
                    stack_arguments.back().format->emplace_back(std::move(arg));
                }
                prev_pos = pos+1;
            } else if (ch == ':') {
                // if (stack_arguments.empty()) {
                //     throw std::runtime_error("Unexpected ':'");
                // }
                // if (stack_arguments.back().index != 0) {
                //     throw std::runtime_error("Unexpected ':'");
                // }
                stack_arguments.back().index = get_index(pos);
                prev_pos = pos+1;
            }
        }
        add_string_to_result(text.substr(prev_pos));
        // if (!stack_arguments.empty()) {
        //     throw std::runtime_error("Missing '}'");
        // }
        return result;
    }
    constexpr std::string parse(auto... args) const {
        auto tree = this->build_tree();
        
        //Evaluate every argument
        std::vector<std::string> evaluated_arguments;
        (evaluated_arguments.emplace_back(std::string_view{args}), ...);
        std::string result;
        for (auto&& node : tree) {
            if (node.plain_text.has_value()) {
                result += node.plain_text->text;
            } else if (node.argument.has_value()) {
                result += evaluated_arguments[node.argument->index];
            }
        }
        return result;
    }
    constexpr auto length(auto... args) const -> size_t {
        return this->parse(args...).length();
    }

    std::string_view text;
};

template <strlit::StringType to_parse, strlit::StringType... Args>
struct Format : strlit::details::BaseString<Parser{to_parse}.length(Args...)> {
    constexpr Format() : strlit::details::BaseString<Parser{to_parse}.length(Args...)>() {
        constexpr auto parser = Parser{to_parse};
        auto result = parser.parse(Args...);
        std::copy(result.begin(), result.end(), this->text);
    }
};