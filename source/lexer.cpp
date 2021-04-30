#include <iostream>
#include <istream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#include "fmt/format.h"
#include "lexer.hpp"
#include "utility.hpp"

const char* token_type_repr(Token::Type type)
{

	switch (type) {
	case (Token::Type::NUMBER):
		return "NUMBER";
	case (Token::Type::STRING):
		return "STRING";
	case (Token::Type::KEYWORD):
		return "KEYWORD";
	case (Token::Type::COMMA):
		return "COMMA";
	case (Token::Type::COLON):
		return "COLON";
	case (Token::Type::OBJECT_OPEN):
		return "OBJECT_OPEN";
	case (Token::Type::OBJECT_CLOSE):
		return "OBJECT_CLOSE";
	case (Token::Type::ARRAY_OPEN):
		return "ARRAY_OPEN";
	case (Token::Type::ARRAY_CLOSE):
		return "ARRAY_CLOSE";
	case (Token::Type::DONE):
		return "EOF";
	default:
		ASSERT_UNREACHABLE();
	}
}

std::string Token::repr() const
{
	std::string result = "Token { .lexeme: ";
	result += "Token { .lexeme: ";
	result += fmt::format("`{}` type: ", this->lexeme);
	result += token_type_repr(this->type);
	result += " }";
	return result;
}

static constexpr std::array<std::pair<char, Token::Type>, 6> single_char_types {
	{
		{ '{', Token::Type::OBJECT_OPEN },
		{ '}', Token::Type::OBJECT_CLOSE },
		{ '[', Token::Type::ARRAY_OPEN },
		{ ']', Token::Type::ARRAY_CLOSE },
		{ ',', Token::Type::COMMA },
		{ ':', Token::Type::COLON },
	}
};

// is the character a valid number start sequence
inline bool could_be_number_start(char c) { return isdigit(c) || c == '-'; }
// is the character a valid part of a number
inline bool could_be_number_internal(char c) { return isdigit(c) || c == '.'; }

class LexerEOF : public std::exception {
};

char read_from_stream(std::istream& stream)
{
	// reads from stream, handling escape characters
	int next = stream.get();
	if (next == EOF) {
		throw LexerEOF();
	}
	if (next == '\\') {
		stream.ignore(1);
		return read_from_stream(stream);
	}
	return static_cast<char>(next);
}

// next token, but doesn't check for EOF
std::unique_ptr<Token> naive_next_token(std::istream& stream)
{
	auto token = std::make_unique<Token>();
	char next_char = read_from_stream(stream);

	// always skip whitespace
	while (isspace(next_char)) {
		next_char = read_from_stream(stream);
	}

	// handle single-char tokens
	static constexpr auto single_char_map
		= ConstMap<char, Token::Type, single_char_types.size()> {
			  { single_char_types }
		  };
	auto const maybe_value = single_char_map.maybe_at(next_char);

	if (maybe_value.has_value()) {
		token->lexeme.push_back(next_char);
		token->type = maybe_value.value();
		return token;
	}

	// handle multi-char tokens
	if (could_be_number_start(next_char)) {
		token->lexeme.push_back(next_char);
		token->type = Token::Type::NUMBER;
		next_char = read_from_stream(stream);
		while (could_be_number_internal(next_char)) {
			if (next_char == '.') {
				token->advice.is_floating_point = true;
			}
			token->lexeme.push_back(next_char);
			next_char = read_from_stream(stream);
		}
		// put back the char that ended the number
		stream.putback(next_char);

		return token;
	}

	if (next_char == '"') {
		token->type = Token::Type::STRING;
		next_char = read_from_stream(stream);

		while (next_char != '"') {
			token->lexeme.push_back(next_char);
			next_char = read_from_stream(stream);
		}
		// note: intentionally consuming trailing quotation mark
		return token;
	}

	if (isalpha(next_char)) {
		token->type = Token::Type::KEYWORD;
		token->lexeme.push_back(next_char);
		next_char = read_from_stream(stream);
		while (isalpha(next_char)) {
			token->lexeme.push_back(next_char);
			next_char = read_from_stream(stream);
		}
		stream.putback(next_char);
		return token;
	}

	std::cout << fmt::format("unreachable with char {}", next_char) << "\n";
	ASSERT_UNREACHABLE();
}

std::unique_ptr<Token> next_token(std::istream& stream)
{
	try {
		return naive_next_token(stream);
	} catch (const LexerEOF& e) {
		auto eof_token = std::make_unique<Token>();
		eof_token->type = Token::Type::DONE;
		eof_token->lexeme = "";
		return eof_token;
	}
}
