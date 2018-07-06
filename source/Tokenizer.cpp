#include "lexer/Tokenizer.h"
#include "lexer/Exception.h"

#include <assert.h>

namespace lexer
{

TokenizerState::TokenizerState(const char* begin, const char* end,
	                           const std::string& escapable_chars, const char escape_char)
	: m_begin(begin)
	, m_cur(m_begin)
	, m_end(end)
	, m_escapable_chars(escapable_chars)
	, m_escape_char(escape_char)
	, m_line(1)
	, m_column(1)
	, m_escaped(false)
{
}

size_t TokenizerState::Length() const
{
    return static_cast<size_t>(m_end - m_begin);
}

const char* TokenizerState::Begin() const
{
    return m_begin;
}

const char* TokenizerState::End() const
{
    return m_end;
}

const char* TokenizerState::CurPos() const
{
    return m_cur;
}

char TokenizerState::CurChar() const
{
    return *m_cur;
}

char TokenizerState::LookAhead(const size_t offset) const
{
	if (Eof(m_cur + offset)) {
		return 0;
	}
    return *(m_cur + offset);
}

size_t TokenizerState::Line() const
{
    return m_line;
}

size_t TokenizerState::Column() const
{
    return m_column;
}

bool TokenizerState::Escaped() const
{
    return !Eof() && m_escaped && m_escapable_chars.find(CurChar()) != std::string::npos;
}
//
//std::string TokenizerState::Unescape(const std::string& str)
//{
//    return StringUtils::unescape(str, m_escapable_chars, m_escape_char);
//}

void TokenizerState::ResetEscaped()
{
    m_escaped = false;
}

bool TokenizerState::Eof() const {
    return Eof(m_cur);
}

bool TokenizerState::Eof(const char* ptr) const
{
    return ptr >= m_end;
}

size_t TokenizerState::Offset(const char* ptr) const
{
    assert(ptr >= m_begin);
    return static_cast<size_t>(ptr - m_begin);
}

void TokenizerState::Advance(const size_t offset)
{
	for (size_t i = 0; i < offset; ++i) {
		Advance();
	}
}

void TokenizerState::Advance()
{
    ErrorIfEof();

    switch (CurChar())
	{
        case '\n':
            ++m_line;
            m_column = 1;
            m_escaped = false;
            break;
        default:
            ++m_column;
			if (CurChar() == m_escape_char) {
				m_escaped = !m_escaped;
			} else {
				m_escaped = false;
			}
            break;
    }
    ++m_cur;
}

void TokenizerState::Reset()
{
    m_cur = m_begin;
    m_line = 1;
    m_column = 1;
    m_escaped = false;
}

void TokenizerState::ErrorIfEof() const
{
	if (Eof()) {
		throw ParserException("Unexpected end of file");
	}
}

TokenizerState::Snapshot TokenizerState::GetSnapshot() const
{
    return Snapshot(*this);
}

void TokenizerState::Restore(const Snapshot& snapshot)
{
    snapshot.Restore(*this);
}

}
