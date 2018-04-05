#include "lrlib.h"

namespace {
	Grammar grammar1 = {
		{ "S",{ "E", "$" } },
		{ "E",{ "E", "*", "B" } },
		{ "E",{ "E", "+", "B" } },
		{ "E",{ "B" } },
		{ "B",{ "0" } },
		{ "B",{ "1" } }
	};

	Grammar grammar2 = {
		{ "S",{ "P", "$" } },
		{ "P",{ "(", "E", ")" } },
		{ "E",{ "1" } },
	};

	Grammar grammar3 = {
		{ "S",{ "E", "$" } },
		{ "E",{ "T" } },
		{ "E",{ "E", "+", "T" } },
		{ "T",{ "F" } },
		{ "F",{ "1" } },
		{ "F",{ "(", "E", ")" } }
	};

	Grammar grammar4 = {
		{ "Z",{ "E", "$" } },
		{ "E",{ "{", "T" , "}" } },
		{ "T",{ "1" } }
	};

}

int main()
{

	testGrammar(grammar2, { "S",{ "P", "$" } }, "(1)$");
	return 0;
}
