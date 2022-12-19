#pragma once
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <array>
#include <string_view>
#include <type_traits>

namespace strlit {
    namespace details {
        template<size_t N>
        struct BaseString {
            constexpr BaseString() {
                text[size] = '\0';
            }
            constexpr operator std::string_view() const {
                return std::string_view(text, size);
            }
            constexpr auto sv() const -> std::string_view {
                return std::string_view(text, size);
            }
            constexpr auto length() const -> size_t {
                size_t i = 0;
                while (text[i++] && i < N);
                return i-1;
            }
            static constexpr size_t SIZE = N;
            size_t size = N;
            char text[N+1];
        };
        constexpr size_t string_length(const char* str) {
            auto begin = str;
            while (*str++);
            return str-begin;
        }
        constexpr size_t string_length(std::string_view str) {
            const auto begin = std::begin(str);
            const auto end = std::end(str);
            auto it = begin;
            while (it != end && *it++);
            return std::distance(begin, it);
        }
    }

    template<size_t N>
    struct String : details::BaseString<N-1> {
        constexpr String()
        {}
        constexpr String(const char (&str)[N]) : details::BaseString<N-1>() {
            std::copy_n(str, N-1, this->text);
        }
        constexpr String(const String<N>& other) : details::BaseString<N-1>() {
            std::copy_n(other.text, N-1, this->text);
        }
    };
    
    template<class T>
    concept IsStringType = std::derived_from<std::remove_cvref_t<T>, details::BaseString<T::SIZE>>;
    template <class T>
    static constexpr bool always_false = false;
    template<class T>
    struct StringType;
    template<IsStringType T>
    struct StringType<T> : T
    {
        constexpr StringType(T) : T() {}
    };
    template<auto N>
    struct StringType<String<N>> : String<N>
    {
        constexpr StringType(const char (&str)[N]) : String<N>(str) {}
        constexpr StringType(const String<N>& str) : String<N>(str) {}
    };
    
    template<auto N>
    StringType(const char (&str)[N]) -> StringType<String<N>>;
    template <IsStringType T>
    StringType(T) -> StringType<T>;


    template <StringType... strings>
    struct Concat : details::BaseString<(decltype(strings)::SIZE + ...)> {
        constexpr Concat() : details::BaseString<(strings.size + ...)>() {
            size_t offset = 0;
            ((std::copy_n(strings.text, strings.size, this->text + offset), offset += strings.size), ...);
        }
    };
    template <StringType separator, StringType... strings>
    struct Join : details::BaseString<(decltype(strings)::SIZE + ...) + (sizeof...(strings) - 1) * decltype(separator)::SIZE> {
        template <size_t I, size_t NextI, size_t... Is>
        constexpr auto add_separator(size_t offset) {
            std::copy_n(separator.text, separator.size, this->text + offset + I);
            add_separator<NextI, Is...>(offset + separator.size + I);
        }
        template <size_t I>
        constexpr auto add_separator(size_t) 
        {}
        constexpr Join() : details::BaseString<(strings.size + ...) + (sizeof...(strings) - 1) * separator.size>() {
            size_t offset = 0;
            ((std::copy_n(strings.text, strings.size, this->text + offset), offset += strings.size+separator.size), ...);
            
                add_separator<strings.size...>(0);
        }
    };
    template <StringType separator, StringType strings>
    struct Split {
        static constexpr auto count_separator() -> size_t {
            size_t count = 0;
            auto str = strings.sv();
            auto pos = str.find(separator.sv());
            while (pos != std::string_view::npos) {
                count++;
                pos = str.find(separator.sv(), pos + separator.size);
            }
            return count;
        }
        std::array<std::string_view, count_separator()+1> splitted;
        constexpr Split() {
            auto str = strings.sv();
            auto pos = str.find(separator.sv());
            size_t i = 0;
            while (pos != std::string_view::npos) {
                splitted[i++] = str.substr(0, pos);
                str = str.substr(pos + separator.size);
                pos = str.find(separator.sv());
            }
            splitted[i] = str;
        }
        constexpr auto operator[](size_t i) const {
            return splitted[i];
        }
        constexpr const auto& operator*() const {
            return splitted;
        }
    };
    template <StringType strings>
    struct Reverse : details::BaseString<decltype(strings)::SIZE> {
        constexpr Reverse() : details::BaseString<strings.size>() {
            std::copy_n(strings.text, strings.size, this->text);
            std::reverse(this->text, this->text + strings.size);
        }
    };
    namespace details {
        constexpr auto absolute(auto n) -> int {
            return (std::signed_integral<decltype(n)> && (n<0)) ? -n : n;
        }
        constexpr auto ceil_log10(auto n) -> int {
            n = absolute(n);
            auto count=0;
            while (n > 0) {
                n /= 10;
                ++count;
            }
            return count;
        }
    }
    template <auto N>
        requires (std::integral<decltype(N)>)
    struct Int : details::BaseString<details::ceil_log10(N)+1*(std::signed_integral<decltype(N)> && (N<0))> {
        static constexpr auto value = N;
        constexpr Int() : details::BaseString<details::ceil_log10(N)+1*(std::signed_integral<decltype(N)> && (N<0))>() {
            auto n = details::absolute(N);
            for (int i = this->size-1; i>=0; --i) {
                this->text[i] = '0' + n % 10;
                n /= 10;
            }
            if (std::signed_integral<decltype(N)> && (N<0)) {
                this->text[0] = '-';
            }
        }
    };
    template <>
    struct Int<0> : details::BaseString<1> {
        static constexpr auto value = 0;
        constexpr Int() : details::BaseString<1>() {
            this->text[0] = '0';
        }
    };
    template<auto N>
        requires std::integral<decltype(N)>
    struct StringType<Int<N>> : Int<N> {
        constexpr StringType(Int<N>) : Int<N>() {}
        constexpr StringType(decltype(N)) : Int<N>() {}
    };
    template <StringType Str>
    struct Shrink : details::BaseString<details::string_length(Str)> {
        constexpr Shrink() : details::BaseString<details::string_length(Str)>() {
            std::copy(Str.text, Str.text+Str.size, this->text);
        }
    };
}