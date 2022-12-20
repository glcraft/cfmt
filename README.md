# cfmt: Constexpr portage of {fmt} library

This is a portage of the library [{fmt}](https://fmt.dev/latest/index.html) in compile time using C++20 features.

Although the implementation is different from the {fmt}, the syntax of the format string is the same.

## Supported formatable type 

* integral types
* types convertible to std::string_view
* simple container with formatable value type like array, vector, list... 
  * You need to include cfmt/formatter_container.h
* map-like container with formatable key and value type (eg `std::array<std::pair<std::string_view, int>>`)
  * You need to include include cfmt/formatter_map.h

## Example

See [src/example.cpp](src/example.cpp) for a full working example.