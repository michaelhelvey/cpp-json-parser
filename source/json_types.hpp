#pragma once

#include <istream>
#include <memory>
#include <variant>
#include <vector>

class json_number {
public:
	constexpr json_number()
		: m_value(0)
		, m_type(json_number_t::INT) {};
	explicit constexpr json_number(int i)
		: m_value(i)
		, m_type(json_number_t::INT) {};
	explicit constexpr json_number(double i)
		: m_value(i)
		, m_type(json_number_t::FLOAT) {};
	constexpr json_number& operator=(const json_number& other);

	[[nodiscard]] constexpr int as_int() const;
	[[nodiscard]] constexpr double as_double() const;

	[[nodiscard]] std::string repr() const;

private:
	std::variant<int, double> m_value;
	enum class json_number_t { INT, FLOAT } m_type;
};

enum class json_type_t {
	STRING,
	NUMBER,
	BOOL,
	ARRAY,
	OBJECT,
	NULLPTR,
};

class json_value;

// Represents a vector of pointers to values.  Is not itself a pointer, because
// at this point I'm thinking that it's better to use value semantics if we just
// have a vector of pointers.
using json_array = std::vector<std::unique_ptr<json_value>>;

struct json_key_value_pair {
	std::string key;
	std::unique_ptr<json_value> value;

	[[nodiscard]] std::string repr() const;
};

class json_object {
public:
	json_object();
	[[nodiscard]] const std::vector<json_key_value_pair>& members() const;
	[[nodiscard]] std::string repr() const;

private:
	std::vector<json_key_value_pair> m_members {};
};

using json_value_cpp_type = std::variant<json_number, bool, std::nullptr_t,
	std::string, json_array, json_object>;

class json_value {
public:
	json_value(json_type_t, json_value_cpp_type);
	[[nodiscard]] json_type_t type() const;
	[[nodiscard]] bool as_bool() const;
	[[nodiscard]] std::string as_string() const;
	[[nodiscard]] json_number as_number() const;
	[[nodiscard]] static std::nullptr_t as_null();
	[[nodiscard]] json_array& as_array() const;
	[[nodiscard]] json_object& as_object() const;

	void add_value_to_array(std::unique_ptr<json_value>) const;

	std::string repr() const;

private:
	json_type_t m_type;
	// marked as mutable so that getters can be marked as const.  I'm genuinely
	// not sure if this is correct, although it is logically true that getting a
	// reference to the value as an array or object does not mutate the logical
	// state of a JSON value.
	mutable json_value_cpp_type m_value;
};
