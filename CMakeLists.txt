cmake_minimum_required (VERSION 3.0)

project(nedit-nm CXX)

add_executable(nedit-nm 
	Error.h
	Expression.h
	Context.h
	Context.cpp
	Reader.cpp
	Reader.h
	main.cpp
	Parser.cpp
	Parser.h
	Statement.h
	Token.h
	Tokenizer.cpp
	Tokenizer.h
	Optimizer.cpp
	Optimizer.h
	CodeGenerator.cpp
	CodeGenerator.h
)

set_property(TARGET nedit-nm PROPERTY CXX_STANDARD 14)
set_property(TARGET nedit-nm PROPERTY CXX_EXTENSIONS OFF)
