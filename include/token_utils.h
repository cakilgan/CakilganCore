#ifndef _TOKEN_UTILS_H
#define _TOKEN_UTILS_H
#include "logic_utils.h"
#include <string_utils.h>
#include <string>
#include <vector>
#include <array>
#include <immintrin.h>
#include <xmemory>

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
    struct WordTokenType{
        const char* tokenTypeName;
        bool operator==(const WordTokenType& other) const{
            return tokenTypeName == other.tokenTypeName;
        }
    };
    struct WordToken : Utils::LogicUtils::canStringfy{
      std::vector<Token> characterTokens;
      WordTokenType type;
      WordToken():characterTokens({}), type({"UNKNOWN"}){}
      WordToken(const std::vector<Token>& characterTokens): characterTokens(characterTokens){}
      WordToken(const std::vector<Token>& characterTokens, const WordTokenType& type): characterTokens(characterTokens), type(type){}
      WordToken(const char* fromLiteral): characterTokens(Utils::TokenUtils::Tokenizer::defaultTokenize(fromLiteral)){}
      WordToken(const std::string& fromLiteral): characterTokens(Utils::TokenUtils::Tokenizer::defaultTokenize(fromLiteral)){}
      std::string toString() const override{
        std::string result;
        for(const auto& token : characterTokens){
          result+=token.literalValue;
        }
        return result;
      }
    };
   namespace Tokenizer{
    std::vector<WordToken> spaceAndPunctuationWordTokenize(std::vector<Token> input);

    float basicParseAndEvaulateMathExpression(std::vector<WordToken> tokenizedText);
   }
}




#endif /* _TOKEN_UTILS_H */
