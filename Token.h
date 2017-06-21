
#ifndef TOKEN_H_
#define TOKEN_H_

#include "Context.h"
#include <string>

class Token {
public:
	enum Type {
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
        Concatenate
	};

public:
    Token() : type(Invalid) {
	}

    Token(Type t, const std::string &v, const Context &c) : type(t), value(v), context(c) {
	}

	Type        type;
	std::string value;
    Context     context;
};

#endif
