#include <cstddef>
#include <iostream>
#include <cfmt/format.h>
#include <cfmt/formatter_container.h>
#include <cfmt/formatter_map.h>
#include <string_view>
#include <tuple>


using namespace std;
static constexpr char txt[] = R"(
Integer: {0}
Integer with padding: "{0: >10}"
Integer with padding and sign: "{0: >+10}"
Integer in hex: {0:#x}
String: {1}
Array: {2}
Map: {3}
Map with padding: "{3: >30}"
Map with integer in hex: {3:#x}
)";
static constexpr auto contained = std::array{std::array{1,2,3}, std::array{4,5,6}};
static constexpr auto mapped = std::array{std::pair{"hello", 123}, std::pair{"world", 456}};


int main(int argc, char** argv)
{
    static constexpr auto compile_time_value = strlit::Shrink<cfmt::format<300>(txt, 12345, "foo", contained, mapped)>{};
    cout << "value constexpr:'" << compile_time_value.sv() << "'" << endl;

    auto runtime_value = cfmt::format_runtime(txt, 12345, "foo", contained, mapped);
    cout << "value runtime:'" << runtime_value << "'" << endl;
    
    return 0;
}