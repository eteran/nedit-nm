
#include "Tokenizer.h"
#include "Error.h"
#include "Reader.h"
#include <cctype>
#include <cstring>
#include <fstream>
#include <regex>
#include <string>

namespace {

const std::regex integer_regex(R"((0|[1-9][0-9]*))");
const std::regex identifier_regex(R"([_a-zA-Z$][_a-zA-Z0-9]*)");
const std::regex whitespace_regex(R"([ \f\r\t\b]+|#.+)");

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

	while (true) {

		// consume whitespace and comments until the next token
		while (true) {
			if (!reader.match(whitespace_regex)) {
				break;
			}
		}

		if (reader.eof()) {
			break;
		}

		// NOTE(eteran): these should be ordered by length so that shorter tokens
		// don't get prioritized over longer ones
		if (reader.match("\\\n")) {
			continue;
		} else if (reader.match("++")) {
			tokens_.emplace_back(Token::Increment, "++", reader.index());
		} else if (reader.match("--")) {
			tokens_.emplace_back(Token::Decrement, "--", reader.index());
		} else if (reader.match("<=")) {
			tokens_.emplace_back(Token::LessThanOrEqual, "<=", reader.index());
		} else if (reader.match(">=")) {
			tokens_.emplace_back(Token::GreaterThanOrEqual, ">=", reader.index());
		} else if (reader.match("==")) {
			tokens_.emplace_back(Token::Equal, "==", reader.index());
		} else if (reader.match("!=")) {
			tokens_.emplace_back(Token::NotEqual, "!=", reader.index());
		} else if (reader.match("+=")) {
			tokens_.emplace_back(Token::AddAssign, "+=", reader.index());
		} else if (reader.match("-=")) {
			tokens_.emplace_back(Token::SubAssign, "-=", reader.index());
		} else if (reader.match("*=")) {
			tokens_.emplace_back(Token::MulAssign, "*=", reader.index());
		} else if (reader.match("/=")) {
			tokens_.emplace_back(Token::DivAssign, "/=", reader.index());
		} else if (reader.match("%=")) {
			tokens_.emplace_back(Token::ModAssign, "%=", reader.index());
		} else if (reader.match("&&")) {
			tokens_.emplace_back(Token::LogicalAnd, "&&", reader.index());
		} else if (reader.match("||")) {
			tokens_.emplace_back(Token::LogicalOr, "||", reader.index());
		} else if (reader.match('{')) {
			tokens_.emplace_back(Token::LeftBrace, "{", reader.index());
		} else if (reader.match('}')) {
			tokens_.emplace_back(Token::RightBrace, "}", reader.index());
		} else if (reader.match(')')) {
			tokens_.emplace_back(Token::RightParen, ")", reader.index());
		} else if (reader.match('(')) {
			tokens_.emplace_back(Token::LeftParen, "(", reader.index());
		} else if (reader.match(']')) {
			tokens_.emplace_back(Token::RightBracket, "]", reader.index());
		} else if (reader.match('[')) {
			tokens_.emplace_back(Token::LeftBracket, "[", reader.index());
		} else if (reader.match(';')) {
			tokens_.emplace_back(Token::Semicolon, ";", reader.index());
		} else if (reader.match(',')) {
			tokens_.emplace_back(Token::Comma, ",", reader.index());
		} else if (reader.match('\n')) {
			tokens_.emplace_back(Token::Newline, "\n", reader.index());
		} else if (reader.match('<')) {
			tokens_.emplace_back(Token::LessThan, "<", reader.index());
		} else if (reader.match('>')) {
			tokens_.emplace_back(Token::GreaterThan, ">", reader.index());
		} else if (reader.match('&')) {
			tokens_.emplace_back(Token::BinaryAnd, "&", reader.index());
		} else if (reader.match('|')) {
			tokens_.emplace_back(Token::BinaryOr, "|", reader.index());
		} else if (reader.match('!')) {
			tokens_.emplace_back(Token::Not, "!", reader.index());
		} else if (reader.match('=')) {
			tokens_.emplace_back(Token::Assign, "=", reader.index());
		} else if (reader.match('+')) {
			tokens_.emplace_back(Token::Add, "+", reader.index());
		} else if (reader.match('-')) {
			tokens_.emplace_back(Token::Sub, "-", reader.index());
		} else if (reader.match('*')) {
			tokens_.emplace_back(Token::Mul, "*", reader.index());
		} else if (reader.match('/')) {
			tokens_.emplace_back(Token::Div, "/", reader.index());
		} else if (reader.match('%')) {
			tokens_.emplace_back(Token::Mod, "%", reader.index());
		} else if (reader.match('^')) {
			tokens_.emplace_back(Token::Exponent, "^", reader.index());
		} else {
			// identifiers/keywords
			char ch = reader.peek();
			if (isdigit(ch)) {

				auto number = reader.match(integer_regex);
				if (!number) {
					throw InvalidNumericConstant(reader.index());
				}

				// make sure that this is a valid integer that won't overflow
				// when converted to an integer
				try {
					(void)std::stoi(*number, nullptr, 10);
				} catch (const std::out_of_range &ex) {
					(void)ex;
					throw InvalidNumericConstant(reader.index());
				}

				tokens_.emplace_back(Token::Integer, *number, reader.index());
			} else if (isalpha(ch) || ch == '_' || ch == '$') {

				auto identifier = reader.match(identifier_regex);
				if (!identifier) {
					throw InvalidIdentifier(reader.index());
				}

				if (*identifier == "while") {
					tokens_.emplace_back(Token::While, *identifier, reader.index());
				} else if (*identifier == "define") {
					tokens_.emplace_back(Token::Define, *identifier, reader.index());
				} else if (*identifier == "in") {
					tokens_.emplace_back(Token::In, *identifier, reader.index());
				} else if (*identifier == "for") {
					tokens_.emplace_back(Token::For, *identifier, reader.index());
				} else if (*identifier == "delete") {
					tokens_.emplace_back(Token::Delete, *identifier, reader.index());
				} else if (*identifier == "if") {
					tokens_.emplace_back(Token::If, *identifier, reader.index());
				} else if (*identifier == "else") {
					tokens_.emplace_back(Token::Else, *identifier, reader.index());
				} else if (*identifier == "switch") {
					tokens_.emplace_back(Token::Switch, *identifier, reader.index());
				} else if (*identifier == "break") {
					tokens_.emplace_back(Token::Break, *identifier, reader.index());
				} else if (*identifier == "continue") {
					tokens_.emplace_back(Token::Continue, *identifier, reader.index());
				} else if (*identifier == "return") {
					tokens_.emplace_back(Token::Return, *identifier, reader.index());
				} else {
					tokens_.emplace_back(Token::Identifier, *identifier, reader.index());
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
								throw InvalidEscapeSequence(reader.index());
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
								throw InvalidEscapeSequence(reader.index());
							}
							break;
						default:
							throw InvalidEscapeSequence(reader.index());
						}
					}
					string.push_back(ch);
				}

				tokens_.emplace_back(Token::String, string, reader.index());
			} else {
				throw TokenizationError(reader.index());
			}
		}
	}
}
