
#include "Tokenizer.h"
#include "Error.h"
#include "Input.h"
#include <cctype>
#include <cstring>
#include <fstream>
#include <string>
#include <regex>

namespace {

const std::regex  integer_regex(R"(^(0|[1-9][0-9]*))");
const std::regex  identifier_regex(R"(^[_a-zA-Z$][_a-zA-Z0-9]*)");
const std::string whitespace(" \f\r\t\b");

/**
 * @brief isodigit
 * @param ch
 * @return
 */
bool isodigit(int ch) {
	return ch >= '0' && ch < '8';
}

}

/**
 * @brief Tokenizer::Tokenizer
 * @param filename
 */
Tokenizer::Tokenizer(const std::string &filename) {

	using std::isdigit;
	using std::isalpha;
	using std::isxdigit;

	std::ifstream file(filename);
	if (!file) {
		throw FileNotFound(filename);
	}

    auto source = std::string(std::istreambuf_iterator<char>{file}, {});

    Input input(std::move(source));

	while (!input.eof()) {

		// consume whitespace and comments until the next token
		while (true) {
			input.consume(whitespace);
			if (input.match('#')) {
				while (input.peek() != '\n') {
					input.read();
				}
			} else {
				break;
			}
		}

		// NOTE(eteran): these should be ordered by length so that shorter tokens
		// don't get prioritized over longer ones
		if (input.match("\\\n")) {
			continue;
		} else if (input.match("++")) {
			tokens_.emplace_back(Token::Increment, "++", Context(input));
		} else if (input.match("--")) {
			tokens_.emplace_back(Token::Decrement, "--", Context(input));
		} else if (input.match("<=")) {
			tokens_.emplace_back(Token::LessThanOrEqual, "<=", Context(input));
		} else if (input.match(">=")) {
			tokens_.emplace_back(Token::GreaterThanOrEqual, ">=", Context(input));
		} else if (input.match("==")) {
			tokens_.emplace_back(Token::Equal, "==", Context(input));
		} else if (input.match("!=")) {
			tokens_.emplace_back(Token::NotEqual, "!=", Context(input));
		} else if (input.match("+=")) {
			tokens_.emplace_back(Token::AddAssign, "+=", Context(input));
		} else if (input.match("-=")) {
			tokens_.emplace_back(Token::SubAssign, "-=", Context(input));
		} else if (input.match("*=")) {
			tokens_.emplace_back(Token::MulAssign, "*=", Context(input));
		} else if (input.match("/=")) {
			tokens_.emplace_back(Token::DivAssign, "/=", Context(input));
		} else if (input.match("%=")) {
			tokens_.emplace_back(Token::ModAssign, "%=", Context(input));
		} else if (input.match("&&")) {
			tokens_.emplace_back(Token::LogicalAnd, "&&", Context(input));
		} else if (input.match("||")) {
			tokens_.emplace_back(Token::LogicalOr, "||", Context(input));
		} else if (input.match('{')) {
			tokens_.emplace_back(Token::LeftBrace, "{", Context(input));
		} else if (input.match('}')) {
			tokens_.emplace_back(Token::RightBrace, "}", Context(input));
		} else if (input.match(')')) {
			tokens_.emplace_back(Token::RightParen, ")", Context(input));
		} else if (input.match('(')) {
			tokens_.emplace_back(Token::LeftParen, "(", Context(input));
		} else if (input.match(']')) {
			tokens_.emplace_back(Token::RightBracket, "]", Context(input));
		} else if (input.match('[')) {
			tokens_.emplace_back(Token::LeftBracket, "[", Context(input));
		} else if (input.match(';')) {
			tokens_.emplace_back(Token::Semicolon, ";", Context(input));
		} else if (input.match(',')) {
			tokens_.emplace_back(Token::Comma, ",", Context(input));
		} else if (input.match('\n')) {
			tokens_.emplace_back(Token::Newline, "\n", Context(input));
		} else if (input.match('<')) {
			tokens_.emplace_back(Token::LessThan, "<", Context(input));
		} else if (input.match('>')) {
			tokens_.emplace_back(Token::GreaterThan, ">", Context(input));
		} else if (input.match('&')) {
			tokens_.emplace_back(Token::BinaryAnd, "&", Context(input));
		} else if (input.match('|')) {
			tokens_.emplace_back(Token::BinaryOr, "|", Context(input));
		} else if (input.match('!')) {
			tokens_.emplace_back(Token::Not, "!", Context(input));
		} else if (input.match('=')) {
			tokens_.emplace_back(Token::Assign, "=", Context(input));
		} else if (input.match('+')) {
			tokens_.emplace_back(Token::Add, "+", Context(input));
		} else if (input.match('-')) {
			tokens_.emplace_back(Token::Sub, "-", Context(input));
		} else if (input.match('*')) {
			tokens_.emplace_back(Token::Mul, "*", Context(input));
		} else if (input.match('/')) {
			tokens_.emplace_back(Token::Div, "/", Context(input));
		} else if (input.match('%')) {
			tokens_.emplace_back(Token::Mod, "%", Context(input));
		} else if (input.match('^')) {
			tokens_.emplace_back(Token::Exponent, "^", Context(input));
		} else {
			// identifiers/keywords
			char ch = input.peek();
			if (isdigit(ch)) {

				std::string number;
				if (!input.match(integer_regex, &number)) {
					throw InvalidNumericConstant(Context(input));
				}

				// make sure that this is a valid integer that won't overflow
				// when converted to an integer
				try {
                    (void)std::stoi(number, nullptr, 10);
				} catch (const std::out_of_range &ex) {
					(void)ex;
					throw InvalidNumericConstant(Context(input));
				}

				tokens_.emplace_back(Token::Integer, number, Context(input));
			} else if (isalpha(ch) || ch == '_' || ch == '$') {

				std::string identifier;
				if (!input.match(identifier_regex, &identifier)) {
					throw InvalidIdentifier(Context(input));
				}

				if (identifier == "while") {
					tokens_.emplace_back(Token::While, identifier, Context(input));
				} else if (identifier == "define") {
					tokens_.emplace_back(Token::Define, identifier, Context(input));
				} else if (identifier == "in") {
					tokens_.emplace_back(Token::In, identifier, Context(input));
				} else if (identifier == "for") {
					tokens_.emplace_back(Token::For, identifier, Context(input));
				} else if (identifier == "delete") {
					tokens_.emplace_back(Token::Delete, identifier, Context(input));
				} else if (identifier == "if") {
					tokens_.emplace_back(Token::If, identifier, Context(input));
				} else if (identifier == "else") {
					tokens_.emplace_back(Token::Else, identifier, Context(input));
				} else if (identifier == "switch") {
					tokens_.emplace_back(Token::Switch, identifier, Context(input));
				} else if (identifier == "break") {
					tokens_.emplace_back(Token::Break, identifier, Context(input));
				} else if (identifier == "continue") {
					tokens_.emplace_back(Token::Continue, identifier, Context(input));
				} else if (identifier == "return") {
					tokens_.emplace_back(Token::Return, identifier, Context(input));
				} else {
					tokens_.emplace_back(Token::Identifier, identifier, Context(input));
				}
			} else if (ch == '"') {
				std::string string;

				// consume the leading quote
				input.read();

				while ((ch = input.read()) != '"') {
					if (ch == '\\') {

                        Input backslash = input;

						ch = input.read();
						switch (ch) {
						case '\n':
							continue; // NOTE(eteran): support escaping a literal newline in the middle of a string
                        case '\'': ch = '\'';  break;
                        case '\"': ch = '\"';  break;
                        case '\\': ch = '\\';  break;
                        case 'a': ch = '\a';   break;
                        case 'b': ch = '\b';   break;
                        case 'f': ch = '\f';   break;
                        case 'n': ch = '\n';   break;
                        case 'r': ch = '\r';   break;
                        case 't': ch = '\t';   break;
                        case 'v': ch = '\v';   break;
                        case 'e': ch = '\x1b'; break;
                        case 'x':
                        case 'X':
                            try {
                                std::string hex;

                                while (isxdigit(input.peek())) {
                                    hex.push_back(input.read());
                                }

                                ch = static_cast<char>(std::stoi(hex, nullptr, 16));

                                // NOTE(eteran): this is a quirk in the NEdit macro language
                                // which attempts to actively prevent literal NULs in strings
                                // by simply ignoring the leading backslash and reparsing
                                if(ch == 0) {
                                    input = backslash;
                                    continue;
                                }

							} catch (...) {
                                throw InvalidEscapeSequence(Context(input));
							}
							break;
						case '0':
						case '1':
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
                        case '7':
							try {
                                std::string oct = {ch};

                                while (isodigit(input.peek())) {
                                    oct.push_back(input.read());
                                }

                                ch = static_cast<char>(std::stoi(oct, nullptr, 8));

                                // NOTE(eteran): this is a quirk in the NEdit macro language
                                // which attempts to actively prevent literal NULs in strings
                                // by simply ignoring the leading backslash and reparsing
                                if(ch == 0) {
                                    input = backslash;
                                    continue;
                                }

							} catch (...) {
                                throw InvalidEscapeSequence(Context(input));
							}
							break;
						default:
							throw InvalidEscapeSequence(Context(input));
						}
					}
					string.push_back(ch);
				}

				tokens_.emplace_back(Token::String, string, Context(input));
			} else if (ch == '\0') {
                break;
			} else {
				throw TokenizationError(Context(input));
			}
		}
	}
}
