#include <iostream>
#include <cstfmt/parser.h>

using namespace std;
static constexpr char txt[] = "Hello {1:ab{2}cd} world";
// static constexpr auto parser = Parser{txt};

// static constexpr auto length_plaintext = parser.length_plaintext();
// static constexpr auto number_arguments = parser.number_arguments();
// static constexpr auto formatted = Format<txt>{};

// static_assert(parser.size_text() == 18);
int main(int argc, char** argv)
{
    // cout << "value:'" << formatted.sv() << "'" << endl;
    // cout << "length_plaintext:" << length_plaintext << endl;
    // cout << "number_arguments:" << number_arguments << endl;
    auto lexer = Lexer(txt);
    auto t = lexer.build_tree();

    return 0;
}
//Hello '' world ''!