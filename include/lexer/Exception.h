#pragma once

#include <guard/Exception.h>

namespace lexer
{

class ParserException : public guard::Exception
{
public:
	ParserException(size_t line, size_t column, const std::string& str = "") noexcept
		: guard::Exception("%s [line %d, column %d]\n", str.c_str(), line, column)
	{}
	ParserException(const std::string& str = "") noexcept
		: guard::Exception(str)
	{}

}; // ParserException

}