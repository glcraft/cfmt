#include <cstddef>
#include <iostream>
#include <cstfmt/format.h>
#include <string_view>

using namespace std;
static constexpr char txt[] = "Hello {:->10} {:-<10} world";

static constexpr auto formatted = strlit::Shrink<cfmt::format(txt, "foo", 10)>{};

int main(int argc, char** argv)
{
    cout << "value constexpr:'" << static_cast<std::string_view>(formatted) << "'" << endl;
    auto test = cfmt::format(txt, "foo", "bar");
    cout << "value runtime:'" << test.sv() << "'" << endl;

    // cout << "value:'" << formatted.sv() << "'" << endl;
    // cout << "length_plaintext:" << length_plaintext << endl;
    // cout << "number_arguments:" << number_arguments << endl;
    

    return 0;
}
//Hello '' world ''!