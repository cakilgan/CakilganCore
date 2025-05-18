#include <token_utils.h>

namespace Utils::TokenUtils
{
    constexpr std::array<const char*, 7> tokenTypes = {
        "ALPHABETIC",
        "NUMERIC",
        "PUNCTUATION",
        "WHITESPACE",
        "UNKNOWN",
        "NEWLINE",
        "EOL"
    };
    const char* getEnumName(TokenType tokenType){
        return tokenTypes[static_cast<uint8_t>(tokenType)];
    }
    namespace Tokenizer{
        
        constexpr TokenType make_lookup_entry(char c) noexcept {
            if (c >= '0' && c <= '9') return TokenType::NUMERIC;
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) return TokenType::ALPHABETIC;
        
            switch (c) {
                case ' ': case '\t': case '\r': case '\f': case '\v': return TokenType::WHITESPACE;
                case '\n': return TokenType::NEWLINE;
                case '.': case ',': case ';': case ':': case '!': case '?': case '-': case '\'':
                case '"': case '(': case ')': case '[': case ']': case '{': case '}': case '/':
                case '\\': case '@': case '#': case '$': case '%': case '^': case '&': case '*':
                case '+': case '=': case '<': case '>': case '|': case '~': case '`': case '_':
                    return TokenType::PUNCTUATION;
                default: return TokenType::UNKNOWN;
            }
        }
        
        constexpr auto make_lookup() noexcept {
            std::array<TokenType, 256> arr{};
            for (int i = 0; i < 256; ++i) {
                arr[i] = make_lookup_entry(static_cast<char>(i));
            }
            return arr;
        }
        
        inline constexpr std::array<TokenType, 256> lookup = make_lookup();
        std::vector<Token> defaultTokenize(std::string_view input){
            std::vector<Token> tokens;
            tokens.reserve(input.size());  
            
            const char* ptr = input.data();
            const char* end = ptr + input.size();
            while (ptr != end) {
                const auto c = static_cast<unsigned char>(*ptr++);
                tokens.emplace_back(lookup[c], c);
            }
            
            return tokens;
        }
        void process_chunk(const char* ptr, Token* out) {
            __m128i c = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr));
        
            auto is_between = [](char lo, char hi, __m128i c) -> __m128i {
                return _mm_and_si128(
                    _mm_cmpgt_epi8(c, _mm_set1_epi8(lo - 1)),
                    _mm_cmplt_epi8(c, _mm_set1_epi8(hi + 1))
                );
            };
        
            __m128i is_digit = is_between('0', '9', c);
            __m128i is_upper = is_between('A', 'Z', c);
            __m128i is_lower = is_between('a', 'z', c);
            __m128i is_alpha = _mm_or_si128(is_upper, is_lower);
        
            __m128i is_space = _mm_or_si128(
                _mm_cmpeq_epi8(c, _mm_set1_epi8(' ')),
                _mm_cmpeq_epi8(c, _mm_set1_epi8('\t'))
            );
            __m128i is_newline = _mm_cmpeq_epi8(c, _mm_set1_epi8('\n'));
        
            // mask to bitmask
            int alpha_mask = _mm_movemask_epi8(is_alpha);
            int digit_mask = _mm_movemask_epi8(is_digit);
            int space_mask = _mm_movemask_epi8(is_space);
            int newline_mask = _mm_movemask_epi8(is_newline);
        
            for (int i = 0; i < 16; ++i) {
                TokenType t;
                if ((digit_mask >> i) & 1) t = TokenType::NUMERIC;
                else if ((alpha_mask >> i) & 1) t = TokenType::ALPHABETIC;
                else if ((space_mask >> i) & 1) t = TokenType::WHITESPACE;
                else if ((newline_mask >> i) & 1) t = TokenType::NEWLINE;
                else t = TokenType::UNKNOWN;
        
                out[i] = Token(t, ptr[i]);
            }
        }
       
        std::vector<Token> SIMDTokenize(std::string_view input){
            std::vector<Token> tokens(input.size());
    
            const char* data = input.data();
            size_t size = input.size();
            
            size_t i = 0;
            for (; i + 16 <= size; i += 16) {
                process_chunk(data + i, &tokens[i]);
            }
            
            for (size_t j = 0; j < size - i; ++j) {
                char c = data[i + j];
                TokenType t;
                t  = lookup[c];
                tokens[i + j] = Token(t, c);
            }
            
            return tokens;
        }
    }
}
