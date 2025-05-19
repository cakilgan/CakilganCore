#include <cstring>
#include <map>
#include <token_utils.h>
#include <math_utils.h>

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
            

            __m128i p0 = is_between('!', '/', c);
            __m128i p1 = is_between(':', '?', c);
            __m128i p2 = is_between('[', '`', c);
            __m128i p3 = is_between('{', '~', c);

            __m128i is_punctuation = _mm_or_si128(p0, _mm_or_si128(p1, _mm_or_si128(p2, p3)));


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
            int punctuation_mask = _mm_movemask_epi8(is_punctuation);
        
            for (int i = 0; i < 16; ++i) {
                TokenType t;
                if ((digit_mask >> i) & 1) t = TokenType::NUMERIC;
                else if ((alpha_mask >> i) & 1) t = TokenType::ALPHABETIC;
                else if ((space_mask >> i) & 1) t = TokenType::WHITESPACE;
                else if ((newline_mask >> i) & 1) t = TokenType::NEWLINE;
                else if ((punctuation_mask >> i) & 1) t = TokenType::PUNCTUATION;
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



        bool isAllNum(const std::vector<Utils::TokenUtils::Token>& tokens) {
            for (const auto& token : tokens) {
                if (token.tokenType != Utils::TokenUtils::TokenType::NUMERIC) {
                    return false;
                }
            }
            return true;
        }
        bool isAllAlpha(const std::vector<Utils::TokenUtils::Token>& tokens) {
            for (const auto& token : tokens) {
                if (token.tokenType != Utils::TokenUtils::TokenType::ALPHABETIC) {
                    return false;
                }
            }
            return true;
        }
        std::vector<WordToken> spaceAndPunctuationWordTokenize(std::vector<Token> tokens){
            std::vector<WordToken> words;
            std::vector<Utils::TokenUtils::Token> current;
            int currentToken = 0;
            for (auto& token : tokens) {
                if(token.tokenType==Utils::TokenUtils::TokenType::WHITESPACE||token.tokenType==Utils::TokenUtils::TokenType::PUNCTUATION){
                    if(!current.empty()){
                        if(isAllNum(current)){
                            words.push_back(Utils::TokenUtils::WordToken(current,{"ALL_NUMBER"}));
                        }
                        else if(isAllAlpha(current)){
                            words.push_back(Utils::TokenUtils::WordToken(current,{"ALL_ALPHA"}));
                        }
                        else{
                            words.push_back(Utils::TokenUtils::WordToken(current));
                        }
                    }
                    current.clear();
        
                    current.push_back(token);
                    if(!current.empty()){
                        if(current.size()==1){
                            if(current[0].literalValue=='+'||current[0].literalValue=='-'||current[0].literalValue=='*'||current[0].literalValue=='/'||current[0].literalValue=='%'){
                                words.push_back(Utils::TokenUtils::WordToken(current,{"OPERATOR"}));
                            }else{
                            words.push_back(Utils::TokenUtils::WordToken(current));
                            }
                        }else{
                            words.push_back(Utils::TokenUtils::WordToken(current));
                        }
                        
                    }
                    current.clear();
        
                }else if(currentToken==tokens.size()-1){
                    current.push_back(token);
                    if(!current.empty()){
                        words.push_back(Utils::TokenUtils::WordToken(current));
                    }
                    current.clear();
                }
                else{
                    current.push_back(token);
                }
                currentToken++;
            }

            return words;
        }
        struct Node {
            std::vector<Utils::TokenUtils::WordToken> words;
            std::string value;
            Node* left = nullptr;
            Node* right = nullptr;
            Node(const std::string& v) {
                value = v;
            }
        };
        int lastGlobalOperator(std::vector<Utils::TokenUtils::WordToken>& words,std::string& opresult){
            std::vector<int> locations;
            std::map<int, std::string> ops;
            int pars= 0;
            for(int i=0;i<words.size();i++){
                Utils::TokenUtils::WordToken word = words[i];
                if(word.toString()=="(") pars++;
                if(word.toString()==")") pars--;
        
                if(pars==0){
                    if(word.toString()=="+"){
                        locations.push_back(i);
                        ops[i] = "+";
                    }
                    if(word.toString()=="-"){
                        locations.push_back(i);
                        ops[i] = "-";
                    }
                }
            }
            if(locations.empty()){
                for(int i=0;i<words.size();i++){
                    Utils::TokenUtils::WordToken word = words[i];
                    if(word.toString()=="(") pars++;
                    if(word.toString()==")") pars--;
            
                    if(pars==0){
                        if(word.toString()=="*"){
                            locations.push_back(i);
                            ops[i] = "*";
                        }
                        if(word.toString()=="/"){
                            locations.push_back(i);
                            ops[i] = "/";
                        }
                        if(word.toString()=="%"){
                            locations.push_back(i);
                            ops[i] = "%";
                        }
                    }
                }
            }
            if(!locations.empty()){
                opresult = ops[locations.back()];
                return locations.back();
            }
            else return -1;
        }
        void splitWithOperator(int lastGlobalOperator,std::vector<Utils::TokenUtils::WordToken>& words,std::vector<Utils::TokenUtils::WordToken>& left,std::vector<Utils::TokenUtils::WordToken>& right){
            left = std::vector<Utils::TokenUtils::WordToken>(words.begin(),words.begin()+lastGlobalOperator);
            right = std::vector<Utils::TokenUtils::WordToken>(words.begin()+lastGlobalOperator+1,words.end());
        }
        
        
        // NOTE:: __look
        bool removeReduntantPars(std::vector<Utils::TokenUtils::WordToken>& words) {
            while (true) {
                if (words.size() < 2 || words.front().toString() != "(" || words.back().toString() != ")")
                    return false;
        
                int depth = 0;
                bool valid = true;
        
                for (size_t i = 0; i < words.size(); ++i) {
                    std::string val = words[i].toString();
        
                    if (val == "(") depth++;
                    else if (val == ")") depth--;
        
                    if (depth == 0 && i != words.size() - 1) {
                        valid = false;
                        break;
                    }
                }
        
                if (valid) {
                    words = std::vector<Utils::TokenUtils::WordToken>(words.begin() + 1, words.end() - 1);
                } else {
                    return false;
                }
            }
        }
        
        
        Node* parse(std::vector<Utils::TokenUtils::WordToken>& words){
            bool isRemovedSomething = removeReduntantPars(words);
            
            std::string op;
            int lastGlobalOperatorV = lastGlobalOperator(words,op);
        
        
            std::vector<Utils::TokenUtils::WordToken> left,right;
            if(lastGlobalOperatorV==-1){
                if(isRemovedSomething){
                    parse(words);
                }
                Node* n =  new Node(Utils::StringUtils::vec_to_str(words,""));
                n->words = words;
                return n;
            }else{
                splitWithOperator(lastGlobalOperatorV,words,left,right);
                
                Node* rtrn = new Node(op);
                rtrn->left = parse(left);
                rtrn->right = parse(right);
                
                return rtrn;
            }
        }
        
        void trim(std::string& trimit){
            for(int i=0;i<trimit.size();i++){
                if(trimit[i]==' '){
                    trimit.erase(trimit.begin()+i);
                    i--;
                }
            }
        }

        // ! using trigonometric funcs with radian values
        float evaulate(Node* start){
            
            if(!start->left&&!start->right){
                if(start->value._Starts_with("sin")){
                    std::vector<Utils::TokenUtils::WordToken> temp (start->words.begin()+1,start->words.end());
                    float val = evaulate(parse(temp));
                    return sin(Utils::MathUtils::degtorad(val));
                }
                if(start->value._Starts_with("cos")){
                    std::vector<Utils::TokenUtils::WordToken> temp (start->words.begin()+1,start->words.end());
                    float val = evaulate(parse(temp));
                    return cos(Utils::MathUtils::degtorad(val));
                }
                if(start->value._Starts_with("tan")){
                    std::vector<Utils::TokenUtils::WordToken> temp (start->words.begin()+1,start->words.end());
                    float val = evaulate(parse(temp));
                    return tan(Utils::MathUtils::degtorad(val));
                }
                if(start->value._Starts_with("log")){
                    std::vector<Utils::TokenUtils::WordToken> temp (start->words.begin()+1,start->words.end());
                    float val = evaulate(parse(temp));
                    return log10(val);
                }
                float val = stof(start->value);
                return val;
            }
        
            float left = evaulate(start->left);
            float right = evaulate(start->right);
            if(start->value=="+") return left+right;
            if(start->value=="-") return left-right;
            if(start->value=="*") return left*right;
            if(start->value=="/") return left/right;
            if(start->value=="%") return (int)left%(int)right;
            throw std::exception("invalid operator!");
        }
        float basicParseAndEvaulateMathExpression(std::vector<WordToken> tokenizedText){
            return evaulate(parse(tokenizedText));
        }
    }
}
