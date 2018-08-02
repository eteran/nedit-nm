cmake_minimum_required (VERSION 3.0)

project(nedit-nm CXX)

add_executable(nedit-nm 
	Error.h
	Expression.h
	Context.h
	Context.cpp
	Input.cpp
	Input.h
	main.cpp
	Parser.cpp
	Parser.h
	Statement.h
	Token.h
	Tokenizer.cpp
	Tokenizer.h
	Optimizer.cpp
	Optimizer.h
)

set_property(TARGET nedit-nm PROPERTY CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -W -Wall -Wmissing-field-initializers -Wunused -Wshadow")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3")
