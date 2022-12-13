#pragma once
#include <cstddef>
#include <string_view>
#include <string>
#include <optional>
#include <concepts>
#include "strlit.h"
#include "utils.h"
#include "formatter.h"

namespace cfmt 
{
    namespace details {

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
    }

    template <size_t N = 10000>
    constexpr auto format(std::string_view format_text, auto... args) -> strlit::String<N>;

    constexpr auto format_string(std::string_view format_text, auto... args) -> std::string {
        auto parsed = details::parse(format_text);
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
                    using formatter_t = Formatter<std::remove_cvref_t<decltype(arg_value)>>;
                    if (i++ == arg.id) {
                        auto formatter = formatter_t{};
                        auto descriptor = formatter.parse(arg.format);
                        result.append(formatter.format(arg_value, descriptor));
                        return true;
                    }
                    return false;
                }(args) || ...);
            }
        }
        return result;
    }

    template <size_t N>
    constexpr auto format(std::string_view format_text, auto... args) -> strlit::String<N>
    {
        strlit::String<N> res;
        auto formatted = format_string(format_text, args...);
        std::copy(formatted.begin(), formatted.end(), res.text);
        std::fill(res.text+formatted.length(), res.text+res.size, '\0');
        return res;
    }
}
