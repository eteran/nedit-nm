
#include "Tokenizer.h"
#include "Error.h"
#include "Reader.h"
#include <cctype>
#include <cstring>
#include <fstream>
#include <regex>
#include <string>

namespace {

const std::regex integer_regex(R"(^(0|[1-9][0-9]*))");
const std::regex identifier_regex(R"(^[_a-zA-Z$][_a-zA-Z0-9]*)");
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

	using std::isalpha;
	using std::isdigit;
	using std::isxdigit;

	std::ifstream file(filename);
	if (!file) {
		throw FileNotFound(filename);
	}

	auto source = std::string(std::istreambuf_iterator<char>{file}, {});

	Reader reader(source);

	while (!reader.eof()) {

		// consume whitespace and comments until the next token
		while (true) {
			reader.consume(whitespace);
			if (reader.match('#')) {
				while (reader.peek() != '\n') {
					reader.read();
				}
			} else {
				break;
			}
		}

		// NOTE(eteran): these should be ordered by length so that shorter tokens
		// don't get prioritized over longer ones
		if (reader.match("\\\n")) {
			continue;
		} else if (reader.match("++")) {
			tokens_.emplace_back(Token::Increment, "++", Context(reader));
		} else if (reader.match("--")) {
			tokens_.emplace_back(Token::Decrement, "--", Context(reader));
		} else if (reader.match("<=")) {
			tokens_.emplace_back(Token::LessThanOrEqual, "<=", Context(reader));
		} else if (reader.match(">=")) {
			tokens_.emplace_back(Token::GreaterThanOrEqual, ">=", Context(reader));
		} else if (reader.match("==")) {
			tokens_.emplace_back(Token::Equal, "==", Context(reader));
		} else if (reader.match("!=")) {
			tokens_.emplace_back(Token::NotEqual, "!=", Context(reader));
		} else if (reader.match("+=")) {
			tokens_.emplace_back(Token::AddAssign, "+=", Context(reader));
		} else if (reader.match("-=")) {
			tokens_.emplace_back(Token::SubAssign, "-=", Context(reader));
		} else if (reader.match("*=")) {
			tokens_.emplace_back(Token::MulAssign, "*=", Context(reader));
		} else if (reader.match("/=")) {
			tokens_.emplace_back(Token::DivAssign, "/=", Context(reader));
		} else if (reader.match("%=")) {
			tokens_.emplace_back(Token::ModAssign, "%=", Context(reader));
		} else if (reader.match("&&")) {
			tokens_.emplace_back(Token::LogicalAnd, "&&", Context(reader));
		} else if (reader.match("||")) {
			tokens_.emplace_back(Token::LogicalOr, "||", Context(reader));
		} else if (reader.match('{')) {
			tokens_.emplace_back(Token::LeftBrace, "{", Context(reader));
		} else if (reader.match('}')) {
			tokens_.emplace_back(Token::RightBrace, "}", Context(reader));
		} else if (reader.match(')')) {
			tokens_.emplace_back(Token::RightParen, ")", Context(reader));
		} else if (reader.match('(')) {
			tokens_.emplace_back(Token::LeftParen, "(", Context(reader));
		} else if (reader.match(']')) {
			tokens_.emplace_back(Token::RightBracket, "]", Context(reader));
		} else if (reader.match('[')) {
			tokens_.emplace_back(Token::LeftBracket, "[", Context(reader));
		} else if (reader.match(';')) {
			tokens_.emplace_back(Token::Semicolon, ";", Context(reader));
		} else if (reader.match(',')) {
			tokens_.emplace_back(Token::Comma, ",", Context(reader));
		} else if (reader.match('\n')) {
			tokens_.emplace_back(Token::Newline, "\n", Context(reader));
		} else if (reader.match('<')) {
			tokens_.emplace_back(Token::LessThan, "<", Context(reader));
		} else if (reader.match('>')) {
			tokens_.emplace_back(Token::GreaterThan, ">", Context(reader));
		} else if (reader.match('&')) {
			tokens_.emplace_back(Token::BinaryAnd, "&", Context(reader));
		} else if (reader.match('|')) {
			tokens_.emplace_back(Token::BinaryOr, "|", Context(reader));
		} else if (reader.match('!')) {
			tokens_.emplace_back(Token::Not, "!", Context(reader));
		} else if (reader.match('=')) {
			tokens_.emplace_back(Token::Assign, "=", Context(reader));
		} else if (reader.match('+')) {
			tokens_.emplace_back(Token::Add, "+", Context(reader));
		} else if (reader.match('-')) {
			tokens_.emplace_back(Token::Sub, "-", Context(reader));
		} else if (reader.match('*')) {
			tokens_.emplace_back(Token::Mul, "*", Context(reader));
		} else if (reader.match('/')) {
			tokens_.emplace_back(Token::Div, "/", Context(reader));
		} else if (reader.match('%')) {
			tokens_.emplace_back(Token::Mod, "%", Context(reader));
		} else if (reader.match('^')) {
			tokens_.emplace_back(Token::Exponent, "^", Context(reader));
		} else {
			// identifiers/keywords
			char ch = reader.peek();
			if (isdigit(ch)) {

				Reader::optional<std::string> number = reader.match(integer_regex);
				if (!number) {
					throw InvalidNumericConstant(Context(reader));
				}

				// make sure that this is a valid integer that won't overflow
				// when converted to an integer
				try {
					(void)std::stoi(*number, nullptr, 10);
				} catch (const std::out_of_range &ex) {
					(void)ex;
					throw InvalidNumericConstant(Context(reader));
				}

				tokens_.emplace_back(Token::Integer, *number, Context(reader));
			} else if (isalpha(ch) || ch == '_' || ch == '$') {

				Reader::optional<std::string> identifier = reader.match(identifier_regex);
				if (!identifier) {
					throw InvalidIdentifier(Context(reader));
				}

				if (*identifier == "while") {
					tokens_.emplace_back(Token::While, *identifier, Context(reader));
				} else if (*identifier == "define") {
					tokens_.emplace_back(Token::Define, *identifier, Context(reader));
				} else if (*identifier == "in") {
					tokens_.emplace_back(Token::In, *identifier, Context(reader));
				} else if (*identifier == "for") {
					tokens_.emplace_back(Token::For, *identifier, Context(reader));
				} else if (*identifier == "delete") {
					tokens_.emplace_back(Token::Delete, *identifier, Context(reader));
				} else if (*identifier == "if") {
					tokens_.emplace_back(Token::If, *identifier, Context(reader));
				} else if (*identifier == "else") {
					tokens_.emplace_back(Token::Else, *identifier, Context(reader));
				} else if (*identifier == "switch") {
					tokens_.emplace_back(Token::Switch, *identifier, Context(reader));
				} else if (*identifier == "break") {
					tokens_.emplace_back(Token::Break, *identifier, Context(reader));
				} else if (*identifier == "continue") {
					tokens_.emplace_back(Token::Continue, *identifier, Context(reader));
				} else if (*identifier == "return") {
					tokens_.emplace_back(Token::Return, *identifier, Context(reader));
				} else {
					tokens_.emplace_back(Token::Identifier, *identifier, Context(reader));
				}
			} else if (ch == '"') {
				std::string string;

				// consume the leading quote
				reader.read();

				while ((ch = reader.read()) != '"') {
					if (ch == '\\') {

						Reader backslash = reader;

						ch = reader.read();
						switch (ch) {
						case '\n':
							continue; // NOTE(eteran): support escaping a literal newline in the middle of a string
						case '\'':
							ch = '\'';
							break;
						case '\"':
							ch = '\"';
							break;
						case '\\':
							ch = '\\';
							break;
						case 'a':
							ch = '\a';
							break;
						case 'b':
							ch = '\b';
							break;
						case 'f':
							ch = '\f';
							break;
						case 'n':
							ch = '\n';
							break;
						case 'r':
							ch = '\r';
							break;
						case 't':
							ch = '\t';
							break;
						case 'v':
							ch = '\v';
							break;
						case 'e':
							ch = '\x1b';
							break;
						case 'x':
						case 'X':
							try {
								std::string hex;

								while (isxdigit(reader.peek())) {
									hex.push_back(reader.read());
								}

								ch = static_cast<char>(std::stoi(hex, nullptr, 16));

								// NOTE(eteran): this is a quirk in the NEdit macro language
								// which attempts to actively prevent literal NULs in strings
								// by simply ignoring the leading backslash and reparsing
								if (ch == 0) {
									reader = backslash;
									continue;
								}

							} catch (...) {
								throw InvalidEscapeSequence(Context(reader));
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

								while (isodigit(reader.peek())) {
									oct.push_back(reader.read());
								}

								ch = static_cast<char>(std::stoi(oct, nullptr, 8));

								// NOTE(eteran): this is a quirk in the NEdit macro language
								// which attempts to actively prevent literal NULs in strings
								// by simply ignoring the leading backslash and reparsing
								if (ch == 0) {
									reader = backslash;
									continue;
								}

							} catch (...) {
								throw InvalidEscapeSequence(Context(reader));
							}
							break;
						default:
							throw InvalidEscapeSequence(Context(reader));
						}
					}
					string.push_back(ch);
				}

				tokens_.emplace_back(Token::String, string, Context(reader));
			} else if (ch == '\0') {
				break;
			} else {
				throw TokenizationError(Context(reader));
			}
		}
	}
}
