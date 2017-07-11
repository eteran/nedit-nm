
#include "Input.h"
#include "Error.h"

/**
 * @brief Input::Input
 * @param source
 * @param input
 */
Input::Input(std::string source, std::string input) : source_(std::move(source)), input_(std::move(input)), index_(0), line_(1), column_(1) {
}

/**
 * @brief Input::peek
 * @return
 */
char Input::peek() const {

	if (eof()) {
		return '\0';
	}

	return input_[index_];
}

/**
 * @brief Input::read
 * @return
 */
char Input::read() {

	if (eof()) {
		return '\0';
	}

	char ch = input_[index_++];

	switch (ch) {
	case '\n':
		++line_;
		column_ = 0;
		break;
	default:
		++column_;
	}

	return ch;
}

/**
 * @brief Input::eof
 * @return
 */
bool Input::eof() const {
	return index_ == input_.size();
}

/**
 * @brief Input::consume
 * @param chars
 */
void Input::consume(const std::string &chars) {

	while (!eof()) {
		char ch = peek();

		if (chars.find(ch) == std::string::npos) {
			break;
		}

		read();
	}
}

/**
 * @brief Input::match
 * @param regex
 * @param match
 * @return
 */
bool Input::match(const std::regex &regex, std::string *match) {

	std::cmatch matches;

	const char *first = &input_[index_];
	const char *last  = &input_[input_.size()];

	if (std::regex_search(first, last, matches, regex)) {
		*match = std::string(matches[0].first, matches[0].second);
		column_ += match->size();
		index_ += match->size();
		return true;
	}

	return false;
}

/**
 * @brief Input::match
 * @param s
 * @return
 */
bool Input::match(const std::string &s) {

	if (index_ + s.size() >= input_.size()) {
		return false;
	}

	for (size_t i = 0; i < s.size(); ++i) {
		if (input_[index_ + i] != s[i]) {
			return false;
		}
	}

	column_ += s.size();
	index_ += s.size();
	return true;
}

/**
 * @brief Input::match
 * @param ch
 * @return
 */
bool Input::match(char ch) {

	if (peek() != ch) {
		return false;
	}

	if (ch == '\n') {
		column_ = 0;
		++line_;
	} else {
		++column_;
	}

	++index_;
	return true;
}

/**
 * @brief Input::index
 * @return
 */
size_t Input::index() const {
	return index_;
}

/**
 * @brief Input::line
 * @return
 */
size_t Input::line() const {
	return line_;
}

/**
 * @brief Input::column
 * @return
 */
size_t Input::column() const {
	return column_;
}
