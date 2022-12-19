#include <cstddef>
#include <iostream>
#include <cfmt/format.h>
#include <string_view>
#include <tuple>

using namespace std;
static constexpr char txt[] = "Hello {:-^20} <{:-^#20x}> |{}| world";
static constexpr auto integer = std::array{1,2,3};

static constexpr auto formatted = strlit::Shrink<cfmt::format(txt, "foo", integer)>{};

int main(int argc, char** argv)
{
    cout << "value constexpr:'" << static_cast<std::string_view>(formatted) << "'" << endl;
    auto test = cfmt::format(txt, "foo", integer);
    cout << "value runtime:'" << test.sv() << "'" << endl;
    
    return 0;
}