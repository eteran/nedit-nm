
#ifndef TOKEN_H_
#define TOKEN_H_

#include <string>

class Token {
public:
	enum Type : uint8_t {
		Invalid,

		Add,
		Sub,
		Mul,
		Div,
		Mod,
		Not,
		Increment,
		Decrement,
		LeftBrace,
		RightBrace,

		LeftParen,
		RightParen,
		LeftBracket,
		RightBracket,
		Semicolon,
		AddAssign,
		SubAssign,
		MulAssign,
		DivAssign,
		ModAssign,

		Exponent,
		LogicalAnd,
		LogicalOr,
		BinaryAnd,
		BinaryOr,
		LessThanOrEqual,
		LessThan,
		GreaterThanOrEqual,
		GreaterThan,
		Equal,

		NotEqual,
		Assign,
		While,
		Define,
		For,
		Delete,
		Comma,
		Newline,
		If,
		In,

		Else,
		Switch,
		Break,
		Continue,
		Return,
		Integer,
		String,
		Identifier,
		ArrayIdentifier,
		Concatenate
	};

public:
	Token() = default;

	Token(Type t, const std::string &v, size_t n)
		: type(t), value(v), index(n) {
	}

	Type type = Invalid;
	std::string value;
	size_t index;
};

#endif
