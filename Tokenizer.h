
#ifndef TOKERNIZER_H_
#define TOKERNIZER_H_

#include "Input.h"
#include "Token.h"
#include <vector>

class Tokenizer {
	using const_iterator = std::vector<Token>::const_iterator;

public:
	explicit Tokenizer(const std::string &filename);
	~Tokenizer() = default;

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

public:
	const_iterator begin() const {
		return tokens_.begin();
	}
	const_iterator end() const {
		return tokens_.end();
	}

private:
	std::vector<Token> tokens_;
};

#endif
