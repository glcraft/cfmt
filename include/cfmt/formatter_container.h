#pragma once
#include "formatter.h"
namespace cfmt {
    template <class T>
    concept IsFormatableContainer = requires(T t) {
        std::begin(t);
        std::end(t);
    } && IsFormatable<std::remove_cvref_t<decltype(*std::begin(std::declval<T>()))>>;
    template <IsFormatableContainer T>
    struct Formatter<T> : Formatter<std::string_view> {
        using ValueType = std::remove_cvref_t<decltype(*std::begin(std::declval<T>()))>;
        template <class DescriptorType>
        constexpr auto format(const T& input, const DescriptorType& desc) const -> std::string {
            constexpr std::string_view default_separator = ", ";
            std::string result;
            auto size = std::distance(std::begin(input), std::end(input));
            // auto separator = desc.separator.value_or(default_separator);
            if constexpr (std::convertible_to<ValueType, std::string_view>) {
                size_t total_size = 0;
                for (const auto& item : input) {
                    total_size += std::string_view(item).length();
                }
                result.reserve(total_size + (size-1)*default_separator.length());
            }
            result.append(1, '{');
            auto begin = std::begin(input);
            auto end = std::end(input);
            auto desc_copy = desc;
            desc_copy.fill_align = std::nullopt;
            desc_copy.width = 0;
            if (begin != end) {
                result.append(Formatter<ValueType>{}.format(*begin, desc_copy));
                ++begin;
            }
            for (; begin != end; ++begin) {
                result.append(default_separator);
                result.append(Formatter<ValueType>{}.format(*begin, desc_copy));
            }
            result.append(1, '}');
            return Formatter<std::string_view>::format(result, desc);
        }
    };
}