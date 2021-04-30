#include "json_types.hpp"
#include "lexer.hpp"
#include <istream>
#include <memory>

class JSONParser {
public:
	explicit JSONParser(std::istream& json_stream);
	std::unique_ptr<json_value> parse();

	/* top level token parser */
	void parse_value(json_array& parent_array);
	void parse_value(json_key_value_pair& parent_key_value_pair);
	std::unique_ptr<json_value>parse_keyword();
	std::unique_ptr<json_value>parse_number();
	std::unique_ptr<json_value>parse_string();
	std::unique_ptr<json_value>parse_array();
	std::unique_ptr<json_value>parse_object();

	/* utilities */
	void match(Token::Type type);
	std::unique_ptr<json_value> build_parsed_value();
	void parse_single_object_member(json_object& parent);
	void parse_rest_object_members(json_object& parent);
	void parse_rest_array_members(json_array& parent);

private:
	std::istream& m_stream;
	std::unique_ptr<Token> m_lookahead;
};