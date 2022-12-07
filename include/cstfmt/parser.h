#include <_types/_uint8_t.h>
#include <cstddef>
#include <string_view>
#include <string>
#include <optional>
#include "strlit.h"

struct PlainText {
    std::string text;
};
struct Argument {
    uint8_t index = -1;
    std::vector<std::variant<PlainText, Argument>> format;
};

struct Lexer {
    struct Character {
        char c;
        size_t pos;
        bool escaped = false;
    };
    constexpr Lexer(std::string_view str, size_t pos = 0) : pos(pos), text(str) 
    {}
    constexpr size_t to_int(std::string_view str) const {
        size_t result = 0;
        for (auto c : str) {
            result = result*10 + (c-'0');
        }
        return result;
    }

    constexpr std::optional<Character> next_character() {
        if (pos >= text.length()) {
            return std::nullopt;
        }
        while (pos < text.length()) {
            switch(text[pos]) {
                case '{':
                {
                    // if (pos != text.length()-1 && text[pos+1] == '{') {
                    //     pos += 2;
                    //     return Character{'{', pos-2, true};
                    // }
                    return Character{'{', pos++, false};
                }
                case '}':
                {
                    // if (pos != text.length()-1 && text[pos+1] == '}') {
                    //     pos += 2;
                    //     return Character{'}', pos-2, true};
                    // }
                    return Character{'}', pos++, false};
                }
                case ':':
                    return Character{':', pos++, false};
                default:
                    pos++;
            }
        }
        return std::nullopt;
    }
    constexpr auto build_tree() {
        auto lexer = Lexer(text);
        std::vector<std::variant<PlainText, Argument>> result;
        std::vector<Argument> stack_arguments;
        auto prev_pos = 0;
        auto add_string_to_result = [&](auto&& str) {
            if (!str.empty()) {
                if (stack_arguments.empty()) {
                    result.push_back(PlainText{std::string(str)});
                } else {
                    stack_arguments.back().format.push_back(PlainText{std::string(str)});
                }
            }
        };
        auto get_index = [&](auto pos) {
            if (prev_pos != pos) {
                return to_int(lexer.text.substr(prev_pos, pos-prev_pos));
            } else {
                return current_index++;
            }
        };
        while (auto ch = next_character()) {
            if (ch->c == '{') {
                add_string_to_result(lexer.text.substr(prev_pos, ch->pos-prev_pos));
                stack_arguments.push_back(Argument{});
            } else if (ch->c == '}') {
                // if (stack_arguments.empty()) {
                //     throw std::runtime_error("Unexpected '}'");
                // }
                if (stack_arguments.back().index == static_cast<uint8_t>(-1)) {
                    stack_arguments.back().index = get_index(ch->pos);
                } else {
                    add_string_to_result(lexer.text.substr(prev_pos, ch->pos-prev_pos));
                }
                auto arg = stack_arguments.back();
                stack_arguments.pop_back();
                if (stack_arguments.empty()) {
                    result.push_back(arg);
                } else {
                    stack_arguments.back().format.push_back(arg);
                }
            } else if (ch->c == ':') {
                // if (stack_arguments.empty()) {
                //     throw std::runtime_error("Unexpected ':'");
                // }
                // if (stack_arguments.back().index != 0) {
                //     throw std::runtime_error("Unexpected ':'");
                // }
                stack_arguments.back().index = get_index(ch->pos);
            }
            prev_pos = ch->pos+1;
        }
        add_string_to_result(lexer.text.substr(prev_pos, lexer.pos-prev_pos));
        // if (!stack_arguments.empty()) {
        //     throw std::runtime_error("Missing '}'");
        // }
        return result;
    }
    constexpr void reset() {
        this->pos = 0;
        this->current_index = 0;
    }
    size_t pos = 0;
    size_t current_index = 0;
    std::string_view text;
    // TokenType current_type;
    // std::vector<PartialToken> partial_tokens;
};
#if 0

// template <size_t N>
struct Parser {
    constexpr Parser(std::string_view str) : text(str)
    {}
    constexpr size_t length_plaintext() const {
        auto lexer = Lexer(text);
        size_t len = 0;
        while(auto token = lexer.next_character()) {
            if (token->type == Lexer::TokenType::PlainText) {
                len += token->length();
            }
        }
        return len;
    }
    constexpr size_t number_arguments() const {
        auto lexer = Lexer(text);
        size_t num = 0;
        while(auto token = lexer.next_character()) {
            if (token->type == Lexer::TokenType::Argument) {
                num++;
            }
        }
        return num;
    }
    std::string_view text;
};
template <strlit::StringType to_parse, strlit::StringType... Args>
struct Format : strlit::details::BaseString<Parser{to_parse}.length_plaintext()> {
    constexpr Format() : strlit::details::BaseString<Parser{to_parse}.length_plaintext()>() {
        constexpr auto parser = Parser{to_parse};
        
        auto lexer = Lexer(to_parse);
        size_t pos = 0;
        while(auto token = lexer.next_character()) {
            if (token->type == Lexer::TokenType::PlainText) {
                std::copy_n(to_parse.text+token->begin, token->length(), this->text+pos);
                pos += token->length();
            } else if (token->type == Lexer::TokenType::Argument) {
                // auto arg = std::get<sizeof...(Args)-parser.number_arguments()>(std::forward_as_tuple(Args{}...));
                // std::copy_n(arg.text, arg.size, this->text+pos);
                // pos += arg.size;
                // pos+=1;
            }
        }
    }
};

#endif