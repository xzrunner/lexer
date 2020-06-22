#pragma once

#include "lexer/Token.h"
#include "lexer/Exception.h"

#include <map>
#include <vector>
#include <sstream>

namespace lexer
{

template <typename TokenType>
class Parser
{
protected:
    typedef std::map<TokenType, std::string> TokenNameMap;

private:
    typedef TokenTemplate<TokenType> Token;
    mutable TokenNameMap m_token_names;

public:
    virtual ~Parser() {}

protected:
    bool Check(const TokenType mask, const Token& token) const {
        return token.HasType(mask);
    }

    const Token& Expect(const TokenType mask, const Token& token) const
	{
		if (!Check(mask, token)) {
			assert(0);
			throw ParserException(token.Line(), token.Column(), ExpectString(TokenName(mask), token));
		}
        return token;
    }

    void Expect(const std::string& type_name, const Token& token) const
	{
        const std::string msg = ExpectString(type_name, token);
		assert(0);
        throw ParserException(token.Line(), token.Column(), msg);
    }

    std::string ExpectString(const std::string& expected, const Token& token) const
	{
        std::stringstream msg;
        msg << "Expected " << expected << ", but got " << TokenName(token.GetType());
		if (!token.Data().empty()) {
			msg << " (raw data: '" << token.Data() << "')";
		}
        return msg.str();
    }

protected:
    std::string TokenName(const TokenType mask) const
	{
		if (m_token_names.empty()) {
			m_token_names = TokenNames();
		}

        std::vector<std::string> names;
        for (const auto& entry : m_token_names)
		{
            const TokenType type = entry.first;
            const std::string& name = entry.second;
			if ((mask & type) != 0) {
				names.push_back(name);
			}
        }

		if (names.empty()) {
			return "unknown token type";
		}
		if (names.size() == 1) {
			return names[0];
		} else {
			std::string ret;
			for (auto& name : names) {
				ret += name + ", ";
			}
			return ret;
		}
    }

private:
    virtual TokenNameMap TokenNames() const = 0;

}; // Parser

}