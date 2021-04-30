#include <iostream>
#include "parser.hpp"
#include "fmt/format.h"
#include "lexer.hpp"
#include "utility.hpp"

class parse_exception : std::exception {
public:
	explicit parse_exception(std::string m)
		: m_msg(std::move(m)) {};
	[[nodiscard]] const char* what() const noexcept override
	{
		return m_msg.c_str();
	}

private:
	std::string m_msg;
};

enum class json_keyword_t {
	JSON_NULL,
	JSON_TRUE,
	JSON_FALSE,
};

using namespace std::literals::string_view_literals;
static constexpr std::array<std::pair<std::string_view, json_keyword_t>, 3>
	keyword_defs { { { "null"sv, json_keyword_t::JSON_NULL },
		{ "true"sv, json_keyword_t::JSON_TRUE },
		{ "false"sv, json_keyword_t::JSON_FALSE } } };

JSONParser::JSONParser(std::istream& json_stream)
	: m_stream(json_stream)
	, m_lookahead(next_token(json_stream))
{
}

static constexpr auto keyword_map
	= ConstMap<std::string_view, json_keyword_t, keyword_defs.size()> {
		  { keyword_defs }
	  };

std::unique_ptr<json_value> JSONParser::parse_keyword()
{
	auto const maybe_keyword = keyword_map.maybe_at(m_lookahead->lexeme);
	if (maybe_keyword.has_value()) {
		switch (maybe_keyword.value()) {
		case (json_keyword_t::JSON_FALSE):
			return std::make_unique<json_value>(json_type_t::BOOL, false);
		case (json_keyword_t::JSON_TRUE):
			return std::make_unique<json_value>(json_type_t::BOOL, true);
		case (json_keyword_t::JSON_NULL):
			return std::make_unique<json_value>(json_type_t::NULLPTR, nullptr);
		default:
			ASSERT_UNREACHABLE();
		}
	} else {
		throw parse_exception(
			fmt::format("Unrecognized keyword {} at position {}",
				m_lookahead->lexeme, m_stream.tellg()));
	}
}

std::unique_ptr<json_value> JSONParser::parse_number()
{
	if (m_lookahead->advice.is_floating_point) {
		return std::make_unique<json_value>(
			json_type_t::NUMBER, json_number(std::stof(m_lookahead->lexeme)));
	} else {
		return std::make_unique<json_value>(
			json_type_t::NUMBER, json_number(std::stoi(m_lookahead->lexeme)));
	}
}

std::unique_ptr<json_value> JSONParser::parse_string() {
	return std::make_unique<json_value>(json_type_t::STRING, m_lookahead->lexeme);
}

std::unique_ptr<json_value> JSONParser::parse_array() {
	auto resulting_array = json_array();
	match(Token::Type::ARRAY_OPEN);
	parse_value(resulting_array);
	parse_rest_array_members(resulting_array);
	match(Token::Type::ARRAY_CLOSE);
	return std::make_unique<json_value>(json_type_t::ARRAY, std::move(resulting_array));
}

void JSONParser::parse_rest_array_members(json_array& parent)
{
	if (m_lookahead->type == Token::Type::COMMA) {
		match(Token::Type::COMMA);
		parse_value(parent);
		parse_rest_array_members(parent);
	}
}

std::unique_ptr<json_value> JSONParser::parse_object() {
	auto resulting_object = json_object();
	match(Token::Type::OBJECT_OPEN);
	parse_single_object_member(resulting_object);
	parse_rest_object_members(resulting_object);
	match(Token::Type::OBJECT_CLOSE);
	return std::make_unique<json_value>(json_type_t::OBJECT, std::move(resulting_object));
}

void JSONParser::parse_single_object_member(json_object& parent)
{
	auto result = json_key_value_pair(m_lookahead->lexeme);
	match(Token::Type::STRING);
	match(Token::Type::COLON);
	parse_value(result);
	parent.add_member(std::move(result));
}

void JSONParser::parse_rest_object_members(json_object& parent)
{
	if(m_lookahead->type == Token::Type::COMMA) {
		match(Token::Type::COMMA);
		parse_single_object_member(parent);
		parse_rest_object_members(parent);
	}
}

void JSONParser::match(Token::Type type)
{
	if (m_lookahead->type == type) {
		m_lookahead = next_token(m_stream);
	} else {
		throw parse_exception(
			fmt::format("Unexpected token {}. Expected type {} at position {}",
				m_lookahead->repr(), token_type_repr(type), m_stream.tellg()));
	}
}

std::unique_ptr<json_value> JSONParser::build_parsed_value()
{
	switch (m_lookahead->type) {
	case (Token::Type::KEYWORD): {
		auto result = parse_keyword();
		match(Token::Type::KEYWORD);
		return result;
	}
	case (Token::Type::NUMBER): {
		auto result = parse_number();
		match(Token::Type::NUMBER);
		return result;
	}
	case (Token::Type::STRING): {
		auto result = parse_string();
		match(Token::Type::STRING);
		return result;
	}
	case (Token::Type::OBJECT_OPEN): {
		auto result = parse_object();
		return result;
	}
	case (Token::Type::ARRAY_OPEN): {
		auto result = parse_array();
		return result;
	}
	default:
		ASSERT_UNREACHABLE();
	}
}

void JSONParser::parse_value(json_array& parent)
{
	parent.emplace_back(build_parsed_value());
}

void JSONParser::parse_value(json_key_value_pair& parent)
{
	auto value = build_parsed_value();
	parent.set_value(std::move(value));
}

std::unique_ptr<json_value> JSONParser::parse()
{
	// all parsed values must have a parent, but the root node does not have a
	// parent, because it's root.  Therefore we create a ghost key value pair to
	// hold the root node, which is deallocated at the end of parsing.
	json_key_value_pair root_pair("root");
	while (m_lookahead->type != Token::Type::DONE) {
		parse_value(root_pair);
	}
	return root_pair.release_value();
}
