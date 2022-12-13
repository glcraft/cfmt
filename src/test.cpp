#include <cstddef>
#include <iostream>
#include <cstfmt/format.h>
#include <string_view>
#include <tuple>

using namespace std;
static constexpr char txt[] = "Hello {:-^20} <{:#x}> world";
static constexpr auto integer = 123456;

static constexpr auto formatted = strlit::Shrink<cfmt::format(txt, "foo", integer)>{};

int main(int argc, char** argv)
{
    cout << "value constexpr:'" << static_cast<std::string_view>(formatted) << "'" << endl;
    auto test = cfmt::format(txt, "foo", integer);
    cout << "value runtime:'" << test.sv() << "'" << endl;
    
    return 0;
}