#include <assert.h>
#include <cstdlib>
#include <istream>
#include <list>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "fmt/format.h"
#include "lexer.hpp"
#include "parser.hpp"
#include "utility.hpp"

enum KeywordType {
	JSON_NULL,
	JSON_TRUE,
	JSON_FALSE,
};

// TODO: use keyword_defs to parse keywords into values
using namespace std::literals::string_view_literals;
static constexpr std::array<std::pair<std::string_view, KeywordType>, 3> keyword_defs {
	{ { "null"sv, JSON_NULL }, { "true"sv, JSON_TRUE }, { "false"sv, JSON_FALSE } }
};

class parser_error : public std::exception {
public:
	parser_error(const std::string m)
		: m_msg(m) {};

	const char* what() const noexcept override { return m_msg.c_str(); }

private:
	std::string m_msg;
};

constexpr json_number::json_number(int i)
	: m_value(i)
	, m_type(number_t::INT)
{
}

constexpr json_number::json_number(double f)
	: m_value(f)
	, m_type(number_t::DOUBLE)
{
}

std::string json_number::repr() const
{
	switch (m_type) {
	case (number_t::INT):
		return fmt::format("{}", as_int());
	case (number_t::DOUBLE):
		return fmt::format("{}", as_double());
	default:
		ASSERT_UNREACHABLE();
	}
}

json_number& json_number::operator=(const json_number& other)
{
	m_value = other.m_value;
	m_type = other.m_type;
	return *this;
}

constexpr int json_number::as_int() const { return std::get<int>(m_value); }
constexpr double json_number::as_double() const { return std::get<double>(m_value); }

json_object::json_object()
	: members { std::vector<std::shared_ptr<json_object_member>>() }
{
}

std::string json_object::repr()
{
	std::stringstream result;
	result << "{ ";
	for (size_t i = 0; i < members.size(); i++) {
		result << "" << members.at(i)->repr();
		if (i != members.size() - 1) {
			result << ", ";
		}
	}
	result << " }";
	return result.str();
}

json_value::json_value(json_value_t v, json_type_t type)
	: m_value(std::move(v))
	, m_type(type)
{
}

std::string json_value::as_string() const { return std::get<std::string>(m_value); }

std::shared_ptr<std::vector<json_value>> json_value::as_array() const
{
	return std::get<std::shared_ptr<std::vector<json_value>>>(m_value);
}

std::shared_ptr<json_object> json_value::as_object() const
{
	return std::get<std::shared_ptr<json_object>>(m_value);
}

bool json_value::as_bool() const { return std::get<bool>(m_value); }

std::nullptr_t json_value::as_null() const { return std::get<std::nullptr_t>(m_value); }

json_number json_value::as_number() const { return std::get<json_number>(m_value); }

const char* json_value::type_repr() const
{
	switch (m_type) {
	case json_type_t::ARRAY:
		return "ARRAY";
	case json_type_t::OBJECT:
		return "OBJECT";
	case json_type_t::STRING:
		return "STRING";
	case json_type_t::NUMBER:
		return "NUMBER";
	case json_type_t::BOOL:
		return "BOOL";
	case json_type_t::NULLPTR:
		return "NULLPTR";
	default:
		ASSERT_UNREACHABLE();
	}
}

std::string json_value::repr()
{
	switch (m_type) {
	case json_type_t::ARRAY: {
		std::stringstream members_repr;
		auto value = as_array();
		for (size_t i = 0; i < value->size(); i++) {
			members_repr << value->at(i).repr();
			if (i != value->size() - 1) {
				members_repr << ", ";
			}
		}
		return fmt::format("[ {} ]", members_repr.str());
	}
	case json_type_t::OBJECT: {
		auto value = as_object();
		return value->repr();
	}
	case json_type_t::STRING:
		return fmt::format("\"{}\"", as_string());
	case json_type_t::BOOL:
		return as_bool() ? "true" : "false";
	case json_type_t::NULLPTR:
		return "null";
	case json_type_t::NUMBER:
		return as_number().repr();
	default:
		ASSERT_UNREACHABLE();
	};
}

json_object_member::json_object_member(std::string k, std::unique_ptr<json_value> v)
	: key(k)
	, value(std::move(v))
{
}

std::string json_object_member::repr() { return fmt::format("\"{}\": {}", key, value->repr()); }

void Parser::match(Token::Type type)
{
	if (m_lookahead->type == type) {
		m_lookahead = next_token(m_stream);
	} else {
		throw parser_error(fmt::format(
			"Expected type {} but received {}", token_type_repr(type), m_lookahead->repr()));
	}
}

void Parser::parse_array(std::variant<std::shared_ptr<json_object_member>, json_value> parent)
{
	match(Token::Type::ARRAY_OPEN);
	json_value_t array_ptr = std::make_shared<std::vector<json_value>>();
	std::unique_ptr<json_value> parsed_value
		= std::make_unique<json_value>(array_ptr, json_type_t::ARRAY);
	if (m_lookahead->type == Token::Type::ARRAY_CLOSE) {
		// we have any empty array
		match(Token::Type::ARRAY_CLOSE);
	} else {
		// it's not an empty object, so continue to parse
		parse_array_members(*(parsed_value.get()));
		match(Token::Type::ARRAY_CLOSE);
	}
	if (std::holds_alternative<std::shared_ptr<json_object_member>>(parent)) {
		// 1. Get the parent as a key-value pair
		auto parent_kvp = std::get<std::shared_ptr<json_object_member>>(parent);
		parent_kvp->value = std::move(parsed_value);
	} else {
		// our parent is another array
		auto parent_array = std::get<json_value>(parent);
		parent_array.as_array()->push_back(*(parsed_value.get()));
	}
}

void Parser::parse_array_members(json_value& parent)
{
	parse_value(parent);
	parse_rest_array_members(parent);
}

void Parser::parse_rest_array_members(json_value& parent)
{
	if (m_lookahead->type == Token::Type::COMMA) {
		match(Token::Type::COMMA);
		parse_value(parent);
		parse_rest_array_members(parent);
	}
}

void Parser::add_parsed_value_to_parent(
	std::variant<std::shared_ptr<json_object_member>, json_value> parent,
	std::unique_ptr<json_value> parsed_value)
{
	if (std::holds_alternative<std::shared_ptr<json_object_member>>(parent)) {
		// the parent is a key-value pair
		auto parent_kvp = std::get<std::shared_ptr<json_object_member>>(parent);
		parent_kvp->value = std::move(parsed_value);
	} else {
		// our parent is an array, in which case we just need to
		// add our parse result to the array
		auto parent_array = std::get<json_value>(parent).as_array();
		// This looks really bad.  But I happen to know that this
		// is a pointer to a vlue on the local stack, so it should
		// be safe to de-reference and therefore push onto the
		// vector of array elements (copying it), right?
		parent_array->push_back(*(parsed_value.get()));
	}
}

// FIXME: this function is very repetitive, and can easy be refactored
// to be less awful
void Parser::parse_value(std::variant<std::shared_ptr<json_object_member>, json_value> parent)
{
	switch (m_lookahead->type) {
	case Token::Type::NUMBER: {
		// 1. parse the number as either a float or an int
		std::string number_as_str = m_lookahead->lexeme;
		match(Token::Type::NUMBER);

		json_number result;
		if (m_lookahead->advice.is_floating_point) {
			result = std::atof(number_as_str.c_str());
		} else {
			result = std::atoi(number_as_str.c_str());
		}
		// 2. create a json value from the number
		auto parsed_value_ptr = std::make_unique<json_value>(result, json_type_t::NUMBER);
		add_parsed_value_to_parent(parent, std::move(parsed_value_ptr));
		break;
	}
	case Token::Type::STRING: {
		// create a json_value with the type of our current lexeme
		// 2. create a json value from the number
		auto parsed_value_ptr
			= std::make_unique<json_value>(m_lookahead->lexeme, json_type_t::STRING);
		match(Token::Type::STRING);
		add_parsed_value_to_parent(parent, std::move(parsed_value_ptr));
		break;
	}
	case Token::Type::ARRAY_OPEN:
		// we have an array
		parse_array(parent);
		break;
	case Token::Type::OBJECT_OPEN:
		parse_as_object(parent);
		break;
	case Token::Type::KEYWORD: {
		static constexpr auto keyword_map
			= ConstMap<std::string_view, KeywordType, keyword_defs.size()> { { keyword_defs } };
		auto const maybe_value = keyword_map.maybe_at(m_lookahead->lexeme);
		if (maybe_value.has_value()) {
			switch (maybe_value.value()) {
			case JSON_NULL: {
				auto parsed_value = std::make_unique<json_value>(nullptr, json_type_t::NULLPTR);
				add_parsed_value_to_parent(parent, std::move(parsed_value));
				match(Token::Type::KEYWORD);
				break;
			}
			case JSON_TRUE: {
				auto parsed_value = std::make_unique<json_value>(true, json_type_t::BOOL);
				add_parsed_value_to_parent(parent, std::move(parsed_value));
				match(Token::Type::KEYWORD);
				break;
			}
			case JSON_FALSE: {
				auto parsed_value = std::make_unique<json_value>(false, json_type_t::BOOL);
				add_parsed_value_to_parent(parent, std::move(parsed_value));
				match(Token::Type::KEYWORD);
				break;
			}
			default:
				ASSERT_UNREACHABLE();
			}
		} else {
			throw parser_error(
				fmt::format("Unexpected unrecognized keyword {}", m_lookahead->lexeme));
		}
		break;
	}
	default:
		ASSERT_UNREACHABLE();
	}
}

void Parser::parse_single_object_member(std::shared_ptr<json_object> parent)
{
	auto kvp = std::make_shared<json_object_member>(m_lookahead->lexeme, nullptr);
	parent->members.push_back(kvp);
	match(Token::Type::STRING);
	match(Token::Type::COLON);
	parse_value(kvp);
}

void Parser::parse_rest_object_members(std::shared_ptr<json_object> parent)
{
	if (m_lookahead->type == Token::Type::COMMA) {
		match(Token::Type::COMMA);
		parse_single_object_member(parent);
		parse_rest_object_members(parent);
	}
}

void Parser::parse_object_members(std::shared_ptr<json_object> parent)
{
	// parse object members...
	parse_single_object_member(parent);
	parse_rest_object_members(parent);
}

// if the parent is a json_value, then it semantically MUST be a
// json_value of type ARRAY.  There is probably a way to enforce this
// using Concepts in C++20, but my brain is too small tbh.
void Parser::parse_as_object(std::variant<std::shared_ptr<json_object_member>, json_value> parent)
{
	match(Token::Type::OBJECT_OPEN);
	// 2. Created a shared pointer to a new, blank object
	auto parsed_object_result = std::make_shared<json_object>();
	if (m_lookahead->type == Token::Type::OBJECT_CLOSE) {
		// it's an empty object, so we can return quickly
		match(Token::Type::OBJECT_CLOSE);
	} else {
		// it's not an empty object, so continue to parse
		parse_object_members(parsed_object_result);
		match(Token::Type::OBJECT_CLOSE);
	}

	// now that we've fully parsed our object, we can add it to its
	// parent and return
	if (std::holds_alternative<std::shared_ptr<json_object_member>>(parent)) {
		// 1. Get the parent as a key-value pair
		auto parent_kvp = std::get<std::shared_ptr<json_object_member>>(parent);
		// 2. Create a json value with our parsed object
		auto parsed_value = std::make_unique<json_value>(parsed_object_result, json_type_t::OBJECT);
		parent_kvp->value = std::move(parsed_value);
	} else {
		// our parent is an array, in which case we just need to
		// add our parse result to the array
		auto parent_array = std::get<json_value>(parent).as_array();
		// create in-place a new JSON value with our empty object into the array
		parent_array->emplace_back(parsed_object_result, json_type_t::OBJECT);
	}
}

/*
RR variant:

object -> { kvp_members } | {}
kvp_members -> kvp_member more_kvp_members
more_kvp_members -> COMMA kvp_member more_kvp_members | E
kvp_member -> string COLON value
array -> [ array_members ] | [ ]
array_members -> value more_array_members
more_array_members -> COMMA value more_array_members | E

boolean -> "true" | "false"
null -> "null"
EOV -> any whitespace
value -> string | number | array | object | boolean | null
 */

std::shared_ptr<json_object> Parser::parse()
{
	// JSON objects always have a parent that is either an array, or a
	// key-value pair, except for the root object.  Rather than making
	// a special type for the root object that has no parent, we
	// create root object with an invisible parent ('invisible_root')
	// that goes away after parser returns

	auto invisible_root = std::make_shared<json_object_member>("root", nullptr);

	while (m_lookahead->type != Token::Type::DONE) {
		parse_as_object(invisible_root);
	}

	if (invisible_root->value) {
		return invisible_root->value->as_object();
	} else {
		return nullptr;
	}
}

Parser::Parser(std::istream& json)
	: m_stream(json)
	, m_lookahead(next_token(json))
{
}
