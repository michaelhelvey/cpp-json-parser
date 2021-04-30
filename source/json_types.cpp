#include "json_types.hpp"
#include "fmt/format.h"
#include "utility.hpp"
#include <sstream>
#include <string>
#include <utility>

/* JSON Number Definitions */

constexpr int json_number::as_int() const { return std::get<int>(m_value); }

constexpr double json_number::as_double() const
{
	return std::get<double>(m_value);
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

void json_object::add_member(json_key_value_pair new_member)
{
	m_members.push_back(std::move(new_member));
}

std::string json_object::repr() const
{
	std::stringstream result;
	result << "{ ";
	for (size_t i = 0; i < m_members.size(); i++) {
		result << m_members.at(i).repr();
		if (i != m_members.size() - 1) { result << ", "; };
	}
	result << " }";

	return result.str();
}

const std::vector<json_key_value_pair>& json_object::members() const
{
	return m_members;
}

/* JSON Key-value pair definitions */

std::string json_key_value_pair::repr() const
{
	return fmt::format("\"{}\": {}", m_key, m_value->repr());
}

json_key_value_pair::json_key_value_pair(
	std::string message, std::unique_ptr<json_value> value)
	: m_key(std::move(message))
	, m_value(std::move(value))
{
}

json_key_value_pair::json_key_value_pair(std::string message)
	: m_key(std::move(message))
	, m_value(std::make_unique<json_value>(json_type_t::NULLPTR, nullptr))
{
}

void json_key_value_pair::set_value(std::unique_ptr<json_value> json_value)
{
	m_value = std::move(json_value);
}

const std::string& json_key_value_pair::key() const { return m_key; }
json_value* json_key_value_pair::value() const { return m_value.get(); }

std::unique_ptr<json_value> json_key_value_pair::release_value()
{
	auto released_value_ptr = m_value.release();
	return std::unique_ptr<json_value>(released_value_ptr);
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

std::nullptr_t json_value::as_null()
{
	return std::get<std::nullptr_t>(m_value);
}

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
	std::stringstream result;
	result << "[";
	for (size_t i = 0; i < array.size(); i++) {
		result << array.at(i)->repr();
		if (i != array.size() - 1) { result << ","; };
	}
	result << "]";

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
