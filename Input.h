
#ifndef INPUT_H_
#define INPUT_H_

#include <cstddef>
#include <regex>
#include <string>

class Input {
public:
    Input(std::string input);

public:
	bool eof() const;
	char peek() const;
	char read();
	void consume(const std::string &chars);
	bool match(char ch);
	bool match(const std::string &s);
	bool match(const std::regex &regex, std::string *match);
	size_t index() const;
	size_t line() const;
	size_t column() const;

private:
    std::string input_; // TODO(eteran): would be better as string_view
    size_t      index_  = 0;
    size_t      line_   = 1;
    size_t      column_ = 1;
};

#endif
