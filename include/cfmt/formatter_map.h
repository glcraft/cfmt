#pragma once
#include "formatter.h"
namespace cfmt {
    namespace details {
        template <class T>
        using ValueMapType = std::remove_cvref_t<decltype(*std::begin(std::declval<T>()))>;
        template <class T>
        concept IsFormatablePair = IsFormatable<typename ValueMapType<T>::first_type>
            && IsFormatable<typename ValueMapType<T>::second_type>;
        template <class T>
        concept IsFormatableMap = requires(T t) {
            std::begin(t);
            std::end(t);
        } && IsFormatablePair<T>;
    }
    template <details::IsFormatableMap T>
    struct Formatter<T> : Formatter<std::string_view> {
        using ValueType = details::ValueMapType<T>;
        template <class DescriptorType>
        constexpr auto format(const T& input, const DescriptorType& desc) const -> std::string {
            constexpr std::string_view separator = ", ";
            std::string result;
            auto size = std::distance(std::begin(input), std::end(input));
            // auto separator = desc.separator.value_or(default_separator);
            if constexpr (std::convertible_to<ValueType, std::string_view>) {
                size_t total_size = 0;
                for (const auto& item : input) {
                    total_size += std::string_view(item).length();
                }
                result.reserve(total_size + (size-1)*separator.length());
            }
            
            result.append(1, '{');
            auto begin = std::begin(input);
            auto end = std::end(input);
            auto desc_copy = desc;
            desc_copy.fill_align = std::nullopt;
            desc_copy.width = 0;
            auto append = [&result, desc_copy](const auto& item) {
                constexpr std::string_view separator_key_value = ": ";
                result.append(Formatter<typename ValueType::first_type>{}.format(item.first, desc_copy));
                result.append(separator_key_value);
                result.append(Formatter<typename ValueType::second_type>{}.format(item.second, desc_copy));
            };
            if (begin != end) {
                append(*begin);
                ++begin;
            }
            for (; begin != end; ++begin) {
                result.append(separator);
                append(*begin);
            }
            result.append(1, '}');
            return Formatter<std::string_view>::format(result, desc);
        }
    };
}