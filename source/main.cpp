#include <fstream>
#include <ios>
#include <iostream>

#include "fmt/format.h"
#include "json_types.hpp"
#include "parser.hpp"

// non spec-compliant parts of the JSON parser:
// No exponents or fraction numbers.

int main()
{
	std::ifstream json_file{"bench/canada.json"};
	if (!json_file) {
		std::cout << "I/O Error: Could not open ./bench/canada.json"	<< "\n";
	}

	JSONParser parser(json_file);
	auto result = parser.parse();

	if (result) {
		std::cout << "Parse result: " << result->repr() << "\n";
	} else {
		std::cout << "Parse result: null" << "\n";
	}


	json_value value(json_type_t::ARRAY, json_array());
	auto inner_num_ptr
		= std::make_unique<json_value>(json_type_t::NUMBER, json_number(10.1));
	value.add_value_to_array(std::move(inner_num_ptr));

	return 0;
}
