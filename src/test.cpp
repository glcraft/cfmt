#include <iostream>
#include <cstfmt/parser.h>

using namespace std;
static constexpr char txt[] = "Hello {1} {0} world";
// static constexpr auto parser = Parser{txt};

// static constexpr auto length_plaintext = parser.length_plaintext();
// static constexpr auto number_arguments = parser.number_arguments();
// static constexpr auto len = Parser{txt}.parse("foo", "bar").length();

static constexpr auto formatted = Format<txt, "foo", strlit::Int<10>{}>{};


// static_assert(parser.size_text() == 18);
int main(int argc, char** argv)
{
    // cout << "len:" << len << endl;
    cout << "value:'" << formatted.sv() << "'" << endl;
    auto test = Parser{txt}.parse("foo", "bar");
    // cout << "value:'" << formatted.sv() << "'" << endl;
    // cout << "length_plaintext:" << length_plaintext << endl;
    // cout << "number_arguments:" << number_arguments << endl;
    

    return 0;
}
//Hello '' world ''!