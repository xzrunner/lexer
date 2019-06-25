// Code from https://github.com/kduske/TrenchBroom

#pragma once

#include "lexer/Token.h"

#include <stack>
#include <memory>

namespace lexer
{

class TokenizerState
{
public:
    TokenizerState(const char* begin, const char* end, const std::string& escapable_chars, char escape_char);

    size_t Length() const;
    const char* Begin() const;
    const char* End() const;

    const char* CurPos() const;
    char CurChar() const;

    char LookAhead(const size_t offset = 1) const;

    size_t Line() const;
    size_t Column() const;

    bool Escaped() const;
 //   std::string Unescape(const std::string& str);
    void ResetEscaped();

    bool Eof() const;
    bool Eof(const char* ptr) const;

    size_t Offset(const char* ptr) const;

    void Advance(const size_t offset);
    void Advance();
    void Reset();

    void ErrorIfEof() const;

public:
	class Snapshot
	{
    public:
        Snapshot(const TokenizerState& state)
			: m_cur(state.m_cur)
			, m_line(state.m_line)
			, m_column(state.m_column)
			, m_escaped(state.m_escaped)
		{
		}

        void Restore(TokenizerState& state) const
		{
            state.m_cur     = m_cur;
            state.m_line    = m_line;
            state.m_column  = m_column;
            state.m_escaped = m_escaped;
        }

	private:
		const char* m_cur;
		size_t      m_line;
		size_t      m_column;
		bool        m_escaped;

    }; // Snapshot

public:
	Snapshot GetSnapshot() const;
	void Restore(const Snapshot& snapshot);

private:
	const char* m_begin;
	const char* m_cur;
	const char* m_end;
	std::string m_escapable_chars;
	char        m_escape_char;
	size_t      m_line;
	size_t      m_column;
	bool        m_escaped;

}; // TokenizerState

template <typename TokenType>
class Tokenizer
{
public:
    typedef TokenTemplate<TokenType> Token;

private:
    typedef std::stack<Token> TokenStack;

    typedef std::shared_ptr<TokenizerState> StatePtr;

    class SaveState
	{
    public:
        SaveState(StatePtr state)
			: m_state(state)
			, m_snapshot(m_state->GetSnapshot())
		{
		}

        ~SaveState() {
            m_state->Restore(m_snapshot);
        }

	private:
		StatePtr m_state;
		TokenizerState::Snapshot m_snapshot;

    }; // SaveState

    StatePtr m_state;

    template<typename T> friend class Tokenizer;

public:
    static const std::string& Whitespace() {
        static const std::string whitespace(" \t\n\r");
        return whitespace;
    }

public:
    Tokenizer(const char* begin, const char* end, const std::string& escapable_chars, const char escape_char)
		: m_state(new TokenizerState(begin, end, escapable_chars, escape_char)) {}

    Tokenizer(const std::string& str, const std::string& escapable_chars, const char escape_char)
		: m_state(new TokenizerState(str.c_str(), str.c_str() + str.size(), escapable_chars, escape_char)) {}

    template <typename OtherType>
    Tokenizer(Tokenizer<OtherType>& nestedTokenizer)
		: m_state(nestedTokenizer.m_state) {}

    Tokenizer(const Tokenizer& other)
		: m_state(other.m_state) {}

    virtual ~Tokenizer() {}

    Token NextToken() {
        return EmitToken();
    }

    Token PeekToken() {
        SaveState old_state(m_state);
        return NextToken();
    }

    void SkipToken(const TokenType skipTokens = ~0u) {
        if (PeekToken().HasType(skipTokens)) {
            NextToken();
        }
    }

    std::string ReadRemainder(const TokenType delimiter_type)
	{
        if (Eof())
            return "";

        Token token = PeekToken();
        const char* start_pos = std::begin(token);
        const char* end_pos = start_pos;
        do {
            token = NextToken();
            end_pos = std::end(token);
        } while (PeekToken().HasType(delimiter_type) == 0 && !Eof());

        return std::string(start_pos, static_cast<size_t>(end_pos - start_pos));
    }

    std::string ReadAnyString(const std::string& delims)
	{
		while (IsWhitespace(CurChar())) {
			Advance();
		}
        const char* start_pos = CurPos();
        const char* end_pos = (CurChar() == '"' ? ReadQuotedString() : ReadUntil(delims));
        return std::string(start_pos, static_cast<size_t>(end_pos - start_pos));
    }

    std::string UnescapeString(const std::string& str) const {
        return m_state->unescape(str);
    }

    void Reset() {
        m_state->Reset();
    }

    double Progress() const
	{
		if (Length() == 0) {
			return 0.0;
		}
        const double cur = static_cast<double>(offset(CurPos()));
        const double len = static_cast<double>(Length());
        return cur / len;
    }

    bool Eof() const {
        return m_state->Eof();
    }
public:
    size_t Line() const {
        return m_state->Line();
    }

    size_t Column() const {
        return m_state->Column();
    }

    size_t Length() const {
        return m_state->Length();
    }
public:
    TokenizerState::Snapshot Snapshot() const {
        return m_state->Snapshot();
    }

    void Restore(const TokenizerState::Snapshot& snapshot) {
        m_state->Restore(snapshot);
    }
protected:
    size_t Offset(const char* ptr) const {
        return m_state->Offset(ptr);
    }

    const char* CurPos() const {
        return m_state->CurPos();
    }

    char CurChar() const {
		if (Eof()) {
			return 0;
		}

        return *CurPos();
    }

    char LookAhead(const size_t offset = 1) const {
        return m_state->LookAhead(offset);
    }

    void Advance(const size_t offset) {
        m_state->Advance(offset);
    }

    void Advance() {
        m_state->Advance();
    }

    bool IsDigit(const char c) const {
        return c >= '0' && c <= '9';
    }

    bool IsLetter(const char c) const {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    bool IsWhitespace(const char c) const {
        return IsAnyOf(c, Whitespace());
    }

    bool IsEscaped() const {
        return m_state->Escaped();
    }

    const char* ReadInteger(const std::string& delims)
	{
		if (CurChar() != '+' && CurChar() != '-' && !IsDigit(CurChar())) {
			return nullptr;
		}

        const TokenizerState previous = *m_state;
		if (CurChar() == '+' || CurChar() == '-') {
			Advance();
		}
		while (!Eof() && IsDigit(CurChar())) {
			Advance();
		}
		if (Eof() || IsAnyOf(CurChar(), delims)) {
			return CurPos();
		}

        *m_state = previous;
        return nullptr;
    }

    const char* ReadDecimal(const std::string& delims)
	{
		if (CurChar() != '+' && CurChar() != '-' && CurChar() != '.' && !IsDigit(CurChar())) {
			return nullptr;
		}

        const TokenizerState previous = *m_state;
        if (CurChar() != '.') {
            Advance();
            ReadDigits();
        }

        if (CurChar() == '.') {
            Advance();
            ReadDigits();
        }

        if (CurChar() == 'e') {
            Advance();
            if (CurChar() == '+' || CurChar() == '-' || IsDigit(CurChar())) {
                Advance();
                ReadDigits();
            }
        }

		if (Eof() || IsAnyOf(CurChar(), delims)) {
			return CurPos();
		}

        *m_state = previous;
        return nullptr;
    }

private:
    void ReadDigits() {
        while (!Eof() && IsDigit(CurChar()))
            Advance();
    }

protected:
    const char* ReadUntil(const std::string& delims) {
		while (!Eof() && !IsAnyOf(CurChar(), delims)) {
			Advance();
		}
        return CurPos();
    }

    const char* ReadWhile(const std::string& allow) {
		while (!Eof() && IsAnyOf(CurChar(), allow)) {
			Advance();
		}
        return CurPos();
    }

    const char* ReadQuotedString(const char delim = '"', const std::string& hack_delims = "")
	{
        while (!Eof() && (CurChar() != delim || IsEscaped())) {
            // This is a hack to handle paths with trailing backslashes that get misinterpreted as escaped double quotation marks.
            if (!hack_delims.empty() && CurChar() == '"' && IsEscaped() && hack_delims.find(LookAhead()) != std::string::npos) {
                m_state->ResetEscaped();
                break;
            }
            Advance();
        }
        ErrorIfEof();
        const char* end = CurPos();
        Advance();
        return end;
    }

    const char* DiscardWhile(const std::string& allow) {
		while (!Eof() && IsAnyOf(CurChar(), allow)) {
			Advance();
		}
        return CurPos();
    }

    const char* DiscardUntil(const std::string& delims) {
		while (!Eof() && !IsAnyOf(CurChar(), delims)) {
			Advance();
		}
        return CurPos();
    }

    bool MatchesPattern(const std::string& pattern) const
	{
		if (pattern.empty() || IsEscaped() || CurChar() != pattern[0]) {
			return false;
		}
        for (size_t i = 1; i < pattern.size(); ++i) {
			if (LookAhead(i) != pattern[i]) {
				return false;
			}
        }
        return true;
    }

    const char* DiscardUntilPattern(const std::string& pattern)
	{
		if (pattern.empty()) {
			return CurPos();
		}

		while (!Eof() && !matchesPattern(pattern)) {
			Advance();
		}

		if (Eof()) {
			return m_state->end();
		}

        return CurPos();
    }

    const char* Discard(const std::string& str)
	{
        for (size_t i = 0; i < str.size(); ++i) {
            const char c = LookAhead(i);
			if (c == 0 || c != str[i]) {
				return nullptr;
			}
        }

        Advance(str.size());
        return CurPos();
    }

    void ErrorIfEof() const {
        m_state->ErrorIfEof();
    }

protected:
    bool IsAnyOf(const char c, const std::string& allow) const {
		for (size_t i = 0; i < allow.size(); i++) {
			if (c == allow[i]) {
				return true;
			}
		}
        return false;
    }

    virtual Token EmitToken() = 0;

}; // Tokenizer

}