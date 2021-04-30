#include "parser.hpp"
/* JSON Parser Definitions */

JSONParser::JSONParser(std::istream& json_stream)
	: m_stream(json_stream)
{
}

std::unique_ptr<json_value> JSONParser::parse()
{
	return std::make_unique<json_value>(json_type_t::NULLPTR, nullptr);
}
