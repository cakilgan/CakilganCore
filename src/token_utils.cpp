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
        
            __m128i zero = _mm_set1_epi8('0');
            __m128i nine = _mm_set1_epi8('9');
            __m128i A = _mm_set1_epi8('A');
            __m128i Z = _mm_set1_epi8('Z');
            __m128i a = _mm_set1_epi8('a');
            __m128i z = _mm_set1_epi8('z');
        
            __m128i space_mask = _mm_or_si128(
                _mm_cmpeq_epi8(c, _mm_set1_epi8(' ')),
                _mm_cmpeq_epi8(c, _mm_set1_epi8('\t'))
            );
            __m128i other_spaces = _mm_or_si128(
                _mm_cmpeq_epi8(c, _mm_set1_epi8('\r')),
                _mm_cmpeq_epi8(c, _mm_set1_epi8('\f'))
            );
            __m128i vertical_tab = _mm_cmpeq_epi8(c, _mm_set1_epi8('\v'));
            __m128i is_space = _mm_or_si128(space_mask, _mm_or_si128(other_spaces, vertical_tab));
        
            __m128i is_newline = _mm_cmpeq_epi8(c, _mm_set1_epi8('\n'));
        
            __m128i ge_zero = _mm_cmpeq_epi8(_mm_max_epu8(c, zero), c);   
            __m128i le_nine = _mm_cmpeq_epi8(_mm_min_epu8(c, nine), c);   
            __m128i is_digit = _mm_and_si128(ge_zero, le_nine);
        
            __m128i ge_A = _mm_cmpeq_epi8(_mm_max_epu8(c, A), c);
            __m128i le_Z = _mm_cmpeq_epi8(_mm_min_epu8(c, Z), c);
            __m128i is_upper = _mm_and_si128(ge_A, le_Z);
        
            __m128i ge_a = _mm_cmpeq_epi8(_mm_max_epu8(c, a), c);
            __m128i le_z = _mm_cmpeq_epi8(_mm_min_epu8(c, z), c);
            __m128i is_lower = _mm_and_si128(ge_a, le_z);
        
            __m128i is_alpha = _mm_or_si128(is_upper, is_lower);
        
            auto in_range = [&](int lo, int hi) -> __m128i {
                __m128i ge_lo = _mm_cmpeq_epi8(_mm_max_epu8(c, _mm_set1_epi8(lo)), c);
                __m128i le_hi = _mm_cmpeq_epi8(_mm_min_epu8(c, _mm_set1_epi8(hi)), c);
                return _mm_and_si128(ge_lo, le_hi);
            };
        
            __m128i punc1 = in_range(33, 47);
            __m128i punc2 = in_range(58, 64);
            __m128i punc3 = in_range(91, 96);
            __m128i punc4 = in_range(123, 126);
        
            __m128i is_punc = _mm_or_si128(_mm_or_si128(punc1, punc2), _mm_or_si128(punc3, punc4));
        
            alignas(16) uint8_t digits_mask[16];
            alignas(16) uint8_t alpha_mask[16];
            alignas(16) uint8_t space_mask_arr[16];
            alignas(16) uint8_t newline_mask_arr[16];
            alignas(16) uint8_t punc_mask_arr[16];
        
            _mm_store_si128(reinterpret_cast<__m128i*>(digits_mask), is_digit);
            _mm_store_si128(reinterpret_cast<__m128i*>(alpha_mask), is_alpha);
            _mm_store_si128(reinterpret_cast<__m128i*>(space_mask_arr), is_space);
            _mm_store_si128(reinterpret_cast<__m128i*>(newline_mask_arr), is_newline);
            _mm_store_si128(reinterpret_cast<__m128i*>(punc_mask_arr), is_punc);
        
            for (int i = 0; i < 16; ++i) {
                TokenType t;
                if (digits_mask[i] != 0) {
                    t = TokenType::NUMERIC;
                } else if (alpha_mask[i] != 0) {
                    t = TokenType::ALPHABETIC;
                } else if (space_mask_arr[i] != 0) {
                    t = TokenType::WHITESPACE;
                } else if (newline_mask_arr[i] != 0) {
                    t = TokenType::NEWLINE;
                } else if (punc_mask_arr[i] != 0) {
                    t = TokenType::PUNCTUATION;
                } else {
                    t = TokenType::UNKNOWN;
                }
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
