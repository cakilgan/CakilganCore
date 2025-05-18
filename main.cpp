#include "string_utils.h"
#include <fstream>
#include <test_utils.h>
#include <token_utils.h>
std::string fileContent;
BoltTestResult TokenizerTest_FUNC(BoltTest& self){ 



    std::vector<Utils::TokenUtils::Token> simdTokens,defaultTokens;
    for (int i = 0; i<1000; i++) {
    SCOPE("SIMD tokenize"){
            simdTokens =  Utils::TokenUtils::Tokenizer::SIMDTokenize(fileContent);
    }
    }

    
    for (int i = 0; i<1000; i++) {
    SCOPE("Default tokenize"){        
            defaultTokens =  Utils::TokenUtils::Tokenizer::defaultTokenize(fileContent);
    }
    }

    
    ASSERT_EQ(simdTokens.size(), defaultTokens.size());
    return BoltTestResult::CALCULATED;
}


int main(){

    
    TIME_PROFILER_IS_ON = true;
    COLORIZED_MODE = true;

    std::ofstream out("tokens.txt");
    if (out.is_open()) {
        for (int i = 0; i < 1600000; i++) {
            out << (char)(i % 95 + 32);
        }
        out.close();
    }
    
    std::fstream file ("tokens.txt");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            fileContent += line;
        }
        file.close();
    }

    BOLT_TEST(TokenizerTest,"Tokenizer Test",TokenizerTest_FUNC);

    std::ofstream resultsjsonfile("results.json");
    runTests(std::cout,resultsjsonfile);
    resultsjsonfile.close();

    return 0;
}