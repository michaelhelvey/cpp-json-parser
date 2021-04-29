#include <assert.h>
#include <ctype.h>
#include <istream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "lexer.hpp"
#include "utility.hpp"

const char *token_type_repr(Token::Type type) {
	
	switch (type) {
	case (Token::Type::NUMBER):
		return "NUMBER";
		break;
	case (Token::Type::STRING):
		return "STRING";
		break;
	case (Token::Type::KEYWORD):
		return "KEYWORD";
		break;
	case (Token::Type::COMMA):
		return "COMMA";
		break;
	case (Token::Type::COLON):
		return "COLON";
		break;
	case (Token::Type::OBJECT_OPEN):
		return "OBJECT_OPEN";
		break;
	case (Token::Type::OBJECT_CLOSE):
		return "OBJECT_CLOSE";
		break;
	case (Token::Type::ARRAY_OPEN):
		return "ARRAY_OPEN";
		break;
	case (Token::Type::ARRAY_CLOSE):
		return "ARRAY_CLOSE";
		break;
	case (Token::Type::DONE):
		return "EOF";
		break;
	default:
		ASSERT_UNREACHABLE();
	}
}

std::string Token::repr()
{
	std::stringstream result(std::string("Token { .lexeme: "));
	result << "Token { .lexeme: ";
	result << '`' << this->lexeme << "`, type: ";
	result << token_type_repr(this->type);
	result << " }";
	return result.str();
}

static constexpr std::array<std::pair<char, Token::Type>, 6> single_char_types { {
	{ '{', Token::Type::OBJECT_OPEN },
	{ '}', Token::Type::OBJECT_CLOSE },
	{ '[', Token::Type::ARRAY_OPEN },
	{ ']', Token::Type::ARRAY_CLOSE },
	{ ',', Token::Type::COMMA },
	{ ':', Token::Type::COLON },
} };

// is the character a valid number start sequence
inline bool could_be_number_start(char c) { return isdigit(c) || c == '-'; }
// is the character a valid part of a number
inline bool could_be_number_internal(char c) { return isdigit(c) || c == '.'; }

class LexerEOF: public std::exception {};

char read_from_stream(std::istream& stream) {
	// reads from stream, handling escape characters
	char next = stream.get();
	if (next == EOF) {
		throw LexerEOF();
	}
	if (next == '\\') {
		stream.ignore(1);
		return read_from_stream(stream);
	}
	return next;
}

// next token, but doesn't check for EOF
std::unique_ptr<Token> _naive_next_token(std::istream& stream)
{
	auto token = std::make_unique<Token>();
	int next_char = read_from_stream(stream);

	// always skip whitespace
	while (isspace(next_char)) {
		next_char = read_from_stream(stream);
	}

	// handle single-char tokens
	static constexpr auto single_char_map = ConstMap<char, Token::Type, single_char_types.size()> {
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

	printf("unreachable with char %c\n", next_char);
	ASSERT_UNREACHABLE();
}

std::unique_ptr<Token> next_token(std::istream& stream)
{
	try {
		return _naive_next_token(stream);
	} catch (const LexerEOF &e) {
		auto eof_token = std::make_unique<Token>();
		eof_token->type = Token::Type::DONE;
		eof_token->lexeme = "";
		return eof_token;
	}
}
