
#ifndef CONTEXT_H_
#define CONTEXT_H_

#include <cstddef>
class Input;

class Context {
public:
	Context();
    explicit Context(const Input &input);

public:
    size_t line() const {
        return line_;
    }

    size_t column() const {
        return column_;
    }

private:
    size_t line_;
    size_t column_;
};

#endif
