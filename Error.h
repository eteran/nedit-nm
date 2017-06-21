
#ifndef ERROR_H_
#define ERROR_H_

#include "Token.h"
#include <exception>
#include <string>

class Error : public std::exception {
public:
    virtual ~Error() = default;
public:
    const char *what() const noexcept override = 0;
};

class SyntaxError : public Error {
public:
    explicit SyntaxError(const Token &token) : token_(token) {
    }

public:
    const char *what() const noexcept override = 0;

public:
    size_t line() const { return token_.context.line(); }
    size_t column() const { return token_.context.column(); }
    const Token &token() const { return token_; }

private:
    Token token_;
};

class FunctionDefinedWithinFunction : public SyntaxError {
public:
    explicit FunctionDefinedWithinFunction(const Token &token) : SyntaxError(token) {
    }
public:
    const char *what() const noexcept override {
        return "FunctionDefinedWithinFunction";
    }
};

class MissingIdentifier : public SyntaxError {
public:
    explicit MissingIdentifier(const Token &token) : SyntaxError(token) {
    }
public:
    const char *what() const noexcept override {
        return "MissingIdentifier";
    }
};

class MissingClosingBracket : public SyntaxError {
public:
    explicit MissingClosingBracket(const Token &token) : SyntaxError(token) {
    }
public:
    const char *what() const noexcept override {
        return "MissingClosingBracket";
    }
};

class MissingClosingBrace : public SyntaxError {
public:
    explicit MissingClosingBrace(const Token &token) : SyntaxError(token) {
    }
public:
    const char *what() const noexcept override {
        return "MissingClosingBrace";
    }
};

class MissingOpenBrace : public SyntaxError {
public:
    explicit MissingOpenBrace(const Token &token) : SyntaxError(token) {
    }
public:
    const char *what() const noexcept override {
        return "MissingOpenBrace";
    }
};

class MissingClosingParen : public SyntaxError {
public:
    explicit MissingClosingParen(const Token &token) : SyntaxError(token) {
    }
public:
    const char *what() const noexcept override {
        return "MissingClosingParen";
    }
};

class MissingOpenParen : public SyntaxError {
public:
    explicit MissingOpenParen(const Token &token) : SyntaxError(token) {
    }
public:
    const char *what() const noexcept override {
        return "MissingOpenParen";
    }
};

class MissingNewline : public SyntaxError {
public:
    explicit MissingNewline(const Token &token) : SyntaxError(token) {
    }
public:
    const char *what() const noexcept override {
        return "MissingNewline";
    }
};

class MissingSemicolon : public SyntaxError {
public:
    explicit MissingSemicolon(const Token &token) : SyntaxError(token) {
    }
public:
    const char *what() const noexcept override {
        return "MissingSemicolon";
    }
};

class UnexpectedBrace : public SyntaxError {
public:
    explicit UnexpectedBrace(const Token &token) : SyntaxError(token) {
    }
public:
    const char *what() const noexcept override {
        return "UnexpectedBrace";
    }
};

class UnexpectedBracket : public SyntaxError {
public:
    explicit UnexpectedBracket(const Token &token) : SyntaxError(token) {
    }
public:
    const char *what() const noexcept override {
        return "UnexpectedBracket";
    }
};

class UnexpectedKeyword : public SyntaxError {
public:
    explicit UnexpectedKeyword(const Token &token) : SyntaxError(token) {
    }
public:
    const char *what() const noexcept override {
        return "UnexpectedKeyword";
    }
};

class UnexpectedParen : public SyntaxError {
public:
    explicit UnexpectedParen(const Token &token) : SyntaxError(token) {
    }
public:
    const char *what() const noexcept override {
        return "UnexpectedParen";
    }
};

class UnexpectedStringConstant : public SyntaxError {
public:
    explicit UnexpectedStringConstant(const Token &token) : SyntaxError(token) {
    }
public:
    const char *what() const noexcept override {
        return "UnexpectedStringConstant";
    }
};

class UnexpectedNumericConstant : public SyntaxError {
public:
    explicit UnexpectedNumericConstant(const Token &token) : SyntaxError(token) {
    }
public:
    const char *what() const noexcept override {
        return "UnexpectedNumericConstant";
    }
};

class UnexpectedIdentifier : public SyntaxError {
public:
    explicit UnexpectedIdentifier(const Token &token) : SyntaxError(token) {
    }
public:
    const char *what() const noexcept override {
        return "UnexpectedIdentifier";
    }
};

class InvalidDelete : public SyntaxError {
public:
    explicit InvalidDelete(const Token &token) : SyntaxError(token) {
    }
public:
    const char *what() const noexcept override {
        return "InvalidDelete";
    }
};

class TokenizationError : public Error {
public:
    explicit TokenizationError(const Context &context) : context_(context) {
    }
public:
    const char *what() const noexcept override {
        return "TokenizationError";
    }

public:
    const Context &context() const { return context_; }

private:
    Context context_;
};

class InvalidIdentifier : public TokenizationError {
public:
    explicit InvalidIdentifier(const Context &context) : TokenizationError(context) {
    }
public:
    const char *what() const noexcept override {
        return "InvalidIdentifier";
    }
};

class InvalidNumericConstant : public TokenizationError {
public:
    explicit InvalidNumericConstant(const Context &context) : TokenizationError(context) {
    }
public:
    const char *what() const noexcept override {
        return "InvalidNumericConstant";
    }
};

class InvalidEscapeSequence : public TokenizationError {
public:
    explicit InvalidEscapeSequence(const Context &context) : TokenizationError(context) {
    }
public:
    const char *what() const noexcept override {
        return "InvalidEscapeSequence";
    }
};




#endif
