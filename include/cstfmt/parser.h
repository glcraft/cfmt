#include <cstddef>
#include <string_view>
#include <optional>
#include "strlit.h"

struct SpanPosition {
    ssize_t begin;
    ssize_t end;
};
template <SpanPosition span_pos>
struct SpanString : strlit::details::BaseString<span_pos.end-span_pos.begin> {
    template <size_t N>
    constexpr SpanString(const char (&str)[N]) : strlit::details::BaseString<span_pos.end-span_pos.begin>() {
        std::copy(str+span_pos.begin, str+span_pos.end, this->text);
    }
    static constexpr SpanPosition position = span_pos;
};

struct Lexer {
    enum class TokenType {
        PlainText,
        Argument,
        None
    };
    struct Token {
        size_t begin, end;
        TokenType type;
        constexpr size_t length() const { 
            return end-begin; 
        }
    };
    constexpr Lexer(std::string_view str, size_t pos = 0) : next_type(TokenType::PlainText), pos(pos), text(str) 
    {}

    constexpr std::optional<Token> next() {
        if (pos >= text.length()) {
            return std::nullopt;
        }
        auto prev_pos = this->pos;
        while(this->pos < text.length()) {
            switch(this->text[this->pos]) {
                case '{':
                {
                    auto current_type = this->next_type;
                    if (this->next_type == TokenType::PlainText && this->pos != text.length()-1 && this->text[this->pos+1] == '{') {
                        auto current_pos = this->pos+1;
                        this->pos += 2;
                        return Token{prev_pos, current_pos, current_type};
                    }
                    this->next_type = TokenType::Argument;
                    return Token{prev_pos, this->pos++, current_type};
                }
                case '}':
                {
                    auto current_type = this->next_type;
                    if (this->next_type == TokenType::PlainText && this->pos != text.length()-1 && this->text[this->pos+1] == '}') {
                        auto current_pos = this->pos+1;
                        this->pos += 2;
                        return Token{prev_pos, current_pos, current_type};
                    }
                    this->next_type = TokenType::PlainText;
                    return Token{prev_pos, this->pos++, current_type};
                }
                default:
                    this->pos++;
            }
        }
        return Token{prev_pos, this->pos, this->next_type};
    }
    TokenType next_type;
    size_t pos = 0;
    std::string_view text;
};
// template <size_t N>
struct Parser {
    constexpr Parser(std::string_view str) : text(str)
    {}
    constexpr size_t length_plaintext() const {
        auto lexer = Lexer(text);
        size_t len = 0;
        while(auto token = lexer.next()) {
            if (token->type == Lexer::TokenType::PlainText) {
                len += token->length();
            }
        }
        return len;
    }
    constexpr size_t number_arguments() const {
        auto lexer = Lexer(text);
        size_t num = 0;
        while(auto token = lexer.next()) {
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
        while(auto token = lexer.next()) {
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
