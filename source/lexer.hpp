#pragma once

#include <istream>
#include <memory>
#include <string>

struct TokenAdvice {
	bool is_floating_point = false;
};

struct Token {
	std::string lexeme {};
	enum Type {
		NUMBER,
		STRING,
		KEYWORD,
		COMMA,
		COLON,
		OBJECT_OPEN,
		OBJECT_CLOSE,
		ARRAY_OPEN,
		ARRAY_CLOSE,
		DONE,
	} type;

	// optional advice to the parser from the lexer about the token
	TokenAdvice advice {};

	[[nodiscard]] std::string repr() const;
};

const char *token_type_repr(Token::Type type);

std::unique_ptr<Token> next_token(std::istream& stream);
