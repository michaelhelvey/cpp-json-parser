#include <iostream>
#include <fstream>
#include <ios>

#include "fmt/format.h"
#include "parser.hpp"

// non spec-compliant parts of the JSON parser:
// No exponents or fraction numbers.
// Cannot start with any value -- must start with object

int main(void)
{
	std::ifstream json_file{"bench/canada.json"};
	if (!json_file) {
		std::cout << "I/O Error: Could not open ./bench/canada.json"	<< "\n";
	}

	printf("read from file\n");
	Parser parser(json_file);
	auto result = parser.parse();

	if (result) {
		std::cout << "Parse result: " << result->repr() << "\n";
	} else {
		std::cout << "Parse result: null" << "\n";
	}

	return 0;
}
