#include <istream>
#include <memory>
#include "json_types.hpp"

class JSONParser {
public:
	explicit JSONParser(std::istream& json_stream);
	std::unique_ptr<json_value> parse();

private:
	std::istream& m_stream;
};