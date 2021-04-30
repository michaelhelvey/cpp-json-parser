#include "json_types.hpp"
#include "fmt/format.h"
#include "utility.hpp"
#include <sstream>
#include <string>
#include <utility>

/* JSON Number Definitions */

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

constexpr int json_number::as_int() const { return std::get<int>(m_value); }

constexpr double json_number::as_double() const
{
	return std::get<double>(m_value);
}

constexpr json_number& json_number::operator=(const json_number& other)
{
	m_value = other.m_value;
	m_type = other.m_type;
	return *this;
}

std::string json_number::repr() const
{
	switch (m_type) {
	case (json_number_t::INT):
		return fmt::format("{}", as_int());
	case (json_number_t::FLOAT):
		return fmt::format("{}", as_double());
	default:
		ASSERT_UNREACHABLE();
	}
}

/* JSON Object Definitions */

json_object::json_object()
	: m_members()
{
}

std::string json_object::repr() const
{
	std::stringstream result("{ ");
	for (auto& el : m_members) {
		result << el.repr() << ", ";
	}

	return result.str();
}

const std::vector<json_key_value_pair>& json_object::members() const
{
	return m_members;
}

/* JSON Key-value pair definitions */

std::string json_key_value_pair::repr() const
{
	return fmt::format("\"{}\": {}", key, value->repr());
}

/* JSON Value Definitions */

json_value::json_value(json_type_t type, json_value_cpp_type value)
	: m_type(type)
	, m_value(std::move(value))
{
}

json_type_t json_value::type() const { return m_type; }

bool json_value::as_bool() const { return std::get<bool>(m_value); }

std::string json_value::as_string() const
{
	return std::get<std::string>(m_value);
}

std::nullptr_t json_value::as_null() { return nullptr; }

json_array& json_value::as_array() const
{
	return std::get<json_array>(m_value);
}

json_number json_value::as_number() const
{
	return std::get<json_number>(m_value);
}

json_object& json_value::as_object() const
{
	return std::get<json_object>(m_value);
}

void json_value::add_value_to_array(std::unique_ptr<json_value> value) const
{
	as_array().push_back(std::move(value));
}

std::string json_array_repr(json_array& array)
{
	std::stringstream result("[ ");
	for (auto& el : array) {
		result << el->repr();
	}

	return result.str();
}

std::string json_value::repr() const
{
	switch (m_type) {
	case (json_type_t::STRING):
		return as_string();
	case (json_type_t::NUMBER):
		return as_number().repr();
	case (json_type_t::OBJECT):
		return as_object().repr();
	case (json_type_t::ARRAY):
		return json_array_repr(as_array());
	case (json_type_t::NULLPTR):
		return "null";
	case (json_type_t::BOOL):
		return as_bool() ? "true" : "false";
	default:
		ASSERT_UNREACHABLE();
	}
}
