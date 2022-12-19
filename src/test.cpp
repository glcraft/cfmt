#include <cstddef>
#include <iostream>
#include <cfmt/format.h>
#include <cfmt/formatter_container.h>
#include <cfmt/formatter_map.h>
#include <string_view>
#include <tuple>


using namespace std;
static constexpr char txt[] = "Hello {:-<30} <{:-^#20x}> world";
static constexpr auto contained = std::array{std::array{1,2,3}, std::array{4,5,6}};
static constexpr auto mapped = std::array{std::pair{"hello", 123}, std::pair{"world", 456}};

static constexpr auto formatted = strlit::Shrink<cfmt::format(txt, contained, mapped)>{};

int main(int argc, char** argv)
{
    cout << "value constexpr:'" << static_cast<std::string_view>(formatted) << "'" << endl;
    auto test = cfmt::format(txt, contained, mapped);
    cout << "value runtime:'" << test.sv() << "'" << endl;
    
    return 0;
}