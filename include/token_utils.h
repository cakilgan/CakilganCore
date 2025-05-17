#ifndef _TOKEN_UTILS_H
#define _TOKEN_UTILS_H
#include "logic_utils.h"
#include <string_utils.h>
#include <string>
#include <vector>
#include <array>
#include <immintrin.h>

namespace Utils::TokenUtils{
    enum class TokenType : uint8_t {
        ALPHABETIC,
        NUMERIC,
        PUNCTUATION,
        WHITESPACE,
        UNKNOWN,
        NEWLINE,
        EOL
    };
    
    const char* getEnumName(TokenType tokenType);
    
    struct Token :Utils::LogicUtils::canStringfy{
        TokenType tokenType;
        char literalValue;
        Token(): tokenType(TokenType::UNKNOWN), literalValue(' '){}
        Token(TokenType type,char literalValue): tokenType(type), literalValue(literalValue){}
        std::string toString() const override{ 
            return StringUtils::as_str(getEnumName(tokenType), "::", literalValue);
        }
    };
    
    namespace Tokenizer {
        std::vector<Token> SIMDTokenize(std::string_view input);
        std::vector<Token> defaultTokenize(std::string_view input);
    }
    
}




#endif /* _TOKEN_UTILS_H */
