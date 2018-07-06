// Code from https://github.com/kduske/TrenchBroom

#pragma once

#include <string>

namespace lexer
{

template <typename Type>
class TokenTemplate
{
private:
    Type m_type;
    const char* m_begin;
    const char* m_end;
    size_t m_position;
    size_t m_line;
    size_t m_column;

public:
    TokenTemplate()
		: m_type(0)
		, m_begin(nullptr)
		, m_end(nullptr)
		, m_position(0)
		, m_line(0)
		, m_column(0)
	{
	}

    TokenTemplate(const Type type, const char* begin, const char* end, const size_t position, const size_t line, const size_t column)
		: m_type(type)
		, m_begin(begin)
		, m_end(end)
		, m_position(position)
		, m_line(line)
		, m_column(column)
	{
        assert(end >= begin);
    }

    Type GetType() const {
        return m_type;
    }

    bool HasType(const Type mask) const {
        return (m_type & mask) != 0;
    }

    const char* Begin() const {
        return m_begin;
    }

    const char* End() const {
        return m_end;
    }

    const std::string Data() const {
        return std::string(m_begin, Length());
    }

    size_t Position() const {
        return m_position;
    }

    size_t Length() const {
        return static_cast<size_t>(m_end - m_begin);
    }

    size_t Line() const {
        return m_line;
    }

    size_t Column() const {
        return m_column;
    }

    template <typename T>
    T ToFloat() const
	{
        static const size_t buf_sz = 256;
        static char buffer[buf_sz];
        assert(Length() < buf_sz);

        memcpy(buffer, m_begin, Length());
        buffer[Length()] = 0;
        const T f = static_cast<T>(std::atof(buffer));
        return f;
    }

    template <typename T>
    T ToInteger() const
	{
        static char buffer[64];
        assert(Length() < 64);

        memcpy(buffer, m_begin, Length());
        buffer[Length()] = 0;
        const T i = static_cast<T>(std::atoi(buffer));
        return i;
    }
};

}