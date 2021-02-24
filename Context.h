
#ifndef CONTEXT_H_
#define CONTEXT_H_

#include <cstddef>
class Reader;

class Context {
public:
	Context() = default;
	explicit Context(const Reader &reader);

public:
	size_t line() const {
		return line_;
	}

	size_t column() const {
		return column_;
	}

private:
	size_t line_   = 0;
	size_t column_ = 0;
};

#endif
