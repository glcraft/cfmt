#pragma once
#include <cstddef>
#include <concepts>
#include <string_view>
#include <string>

namespace cfmt::utils {
    constexpr auto sv_to_int(std::string_view str) -> int64_t {
        size_t result = 0;
        auto sign = 1;
        if (str[0] == '-') {
            sign = -1;
            str.remove_prefix(1);
        }
        for (auto c : str) {
            result = result*10 + (c-'0');
        }
        return result * sign;
    }
    constexpr auto to_upper(char c) -> char {
        if (c >= 'a' && c <= 'z') {
            return c - 'a' + 'A';
        }
        return c;
    }
    
}