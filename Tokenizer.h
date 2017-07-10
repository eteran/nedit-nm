
#ifndef TOKERNIZER_H_
#define TOKERNIZER_H_

#include "Input.h"
#include "Token.h"
#include <vector>

class Tokenizer {
public:
	explicit Tokenizer(const std::string &filename);

public:
	size_t size() const {
		return tokens_.size();
	}

public:
	Token operator[](const size_t index) const {
		if (index < tokens_.size()) {
			return tokens_[index];
		} else {
			return Token();
		}
	}

private:
	std::vector<Token> tokens_;
};

#endif
