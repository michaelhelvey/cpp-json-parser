#include <sstream>
#include <fstream>
#include <ios>
#include <iostream>

#include "fmt/format.h"
#include "parser.hpp"

// non spec-compliant parts of the JSON parser:
// No exponents or fraction numbers.
// parsing floating point numbers back to their original numbers goes REALLY
// WELL!! (hehe)

int main()
{
	std::ios_base::sync_with_stdio(false);
	std::ifstream json_file{"bench/canada.json"};
	if (!json_file) {
		std::cout << "I/O Error: Could not open ./bench/canada.json" << "\n";
	}

	JSONParser parser(json_file);
	auto result = parser.parse();

	if (result) {
		std::cout << "Parse result: " << result->repr() << "\n";
	} else {
		std::cout << "Parse result: null" << "\n";
	}

	return 0;
}
