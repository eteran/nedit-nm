
#include "Input.h"
#include "Error.h"
#include <cctype>
#include <iostream>

Input::Input(std::string source, std::string input) : source_(std::move(source)), input_(std::move(input)), index_(0), line_(1), column_(1) {
}

char Input::peek() const {

	if (eof()) {
		return '\0';
	}

	return input_[index_];
}

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

bool Input::eof() const {
	return index_ == input_.size();
}

void Input::consume(const std::string &chars) {

	while (!eof()) {
		char ch = peek();

        if(chars.find(ch) == std::string::npos) {
            break;
        }

		read();
	}
}

bool Input::match(const std::regex &regex, std::string *match) {

	std::cmatch matches;

	const char *first = &input_[index_];
	const char *last  = &input_[input_.size()];

	if (std::regex_search(first, last, matches, regex)) {
		*match = std::string(matches[0].first, matches[0].second);
		column_ += match->size();
		index_  += match->size();
		return true;
	}

	return false;
}

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
	index_  += s.size();
	return true;
}

bool Input::match(char ch) {

    if(peek() != ch) {
        return false;
    }

    if(ch == '\n') {
        column_ = 0;
        ++line_;
    } else {
        ++column_;
    }
    ++index_;
    return true;
}

size_t Input::index() const {
	return index_;
}

size_t Input::line() const {
	return line_;
}

size_t Input::column() const {
	return column_;
}
