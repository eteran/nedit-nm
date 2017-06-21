
#include "Context.h"
#include "Input.h"

/**
 * @brief Context::Context
 */
Context::Context() : line_(0), column_(0) {

}

/**
 * @brief Context::Context
 * @param input
 */
Context::Context(const Input &input) : line_(input.line()), column_(input.column()) {

}
