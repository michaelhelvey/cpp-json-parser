#pragma once
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "lexer.hpp"

enum class json_type_t {
	ARRAY,
	OBJECT,
	STRING,
	NUMBER,
	BOOL,
	NULLPTR,
};

enum class number_t {
	DOUBLE,
	INT
};

class json_number final {
public:
	constexpr json_number(): m_value(0), m_type(number_t::INT) {};
	constexpr json_number(int i);
	constexpr json_number(double f);
	json_number& operator=(const json_number& other);

	constexpr int as_int() const;

	constexpr double as_double() const;

	std::string repr() const;
private:
	std::variant<int, double> m_value;
	number_t m_type;
};

class json_value;

struct json_object_member {
	json_object_member(std::string k, std::unique_ptr<json_value> v);
	const std::string key;
	std::unique_ptr<json_value> value;

	std::string repr();
};

struct json_object {
	json_object();
	std::vector<std::shared_ptr<json_object_member>> members;

	std::string repr();
};

using json_value_t = std::variant<std::shared_ptr<std::vector<json_value>>,
	std::shared_ptr<json_object>, std::string,
	json_number, bool, std::nullptr_t>;

class json_value final {
public:
	json_value(json_value_t v, json_type_t type);

	std::string as_string() const;
	std::shared_ptr<std::vector<json_value>> as_array() const;
	std::shared_ptr<json_object> as_object() const;
	bool as_bool() const;
	std::nullptr_t as_null() const;
	json_number as_number() const;

	const char* type_repr() const;
	std::string repr();

private:
	const json_value_t m_value;
	const json_type_t m_type;
};

class Parser final {
public:
	Parser(std::istream& json);
	std::shared_ptr<json_object> parse();

private:
	std::istream& m_stream;
	std::unique_ptr<Token> m_lookahead;
	json_object root;

	void match(Token::Type type);
	void parse_as_object(std::variant<std::shared_ptr<json_object_member>, json_value> parent);
	void parse_object_members(std::shared_ptr<json_object> parent);
	void parse_single_object_member(std::shared_ptr<json_object> parent);
	void parse_rest_object_members(std::shared_ptr<json_object> parent);
	void parse_array(std::variant<std::shared_ptr<json_object_member>, json_value> parent);
	void parse_array_members(json_value &parent);
	void parse_value(std::variant<std::shared_ptr<json_object_member>, json_value> parent);
	void parse_rest_array_members(json_value &parent);
	void add_parsed_value_to_parent(std::variant<std::shared_ptr<json_object_member>, json_value> parent, std::unique_ptr<json_value> parsed_value);
};
