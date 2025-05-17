#ifndef BOLT_TEST_H
#define BOLT_TEST_H

#include "time_utils.h"
#include <string_utils.h>
#include <color_utils.h>

#include <functional>
#include <string>
#include <vector>
#include <type_traits>
#include <sstream>
inline static bool TIME_PROFILER_IS_ON = false;
inline static bool COLORIZED_MODE = false;

enum class BoltTestResult {
    PASSED = 0,
    FAILED = 1,
    SUS_PASS = -1,
    SUS_FAIL = -2,
    CALCULATED = -3
};


inline const char* getResultName(BoltTestResult result) {
    switch (result) {
    case BoltTestResult::PASSED:
        return "PASSED";
    case BoltTestResult::FAILED:
        return "FAILED";
    case BoltTestResult::SUS_PASS:
        return "SUS_PASS";
    case BoltTestResult::SUS_FAIL:
        return "SUS_FAIL";
    case BoltTestResult::CALCULATED:
        return "CALCULATED";
    default:
        return "UNKNOWN:: ERROR";
    }
}
namespace Utils::ColorUtils{
    inline ANSIIColor getResultColor(BoltTestResult result) {
        switch (result) {
        case BoltTestResult::PASSED:
            return GREEN;
        case BoltTestResult::FAILED:
            return RED;
        case BoltTestResult::SUS_PASS:
            return CYAN;
        case BoltTestResult::SUS_FAIL:
            return YELLOW;
        case BoltTestResult::CALCULATED:
            return BRIGHT_MAGENTA;
        default:
            return BRIGHT_RED;
        }
    }
}

struct BoltTestEvent{
    BoltTestResult result;
    std::string message;
};
struct BoltTest {
    private:
    BoltTestResult result;
  public:
  inline BoltTestResult getResult() { return result; }
  inline void setResult(BoltTestResult result) { this->result = result; }
  inline void addEvent(BoltTestEvent event) { events.push_back(event); }
  inline void addEvent(std::string message, BoltTestResult result) { addEvent(BoltTestEvent{ result, message }); }
  inline void flush() { events.clear(); contexts.clear(); result = BoltTestResult::CALCULATED;}
  const char* name;
  const char* description;
  std::vector<BoltTestEvent> events;
  std::vector<std::string> contexts;
  std::function<BoltTestResult(BoltTest& test)> test;  
};

inline thread_local BoltTest* CURRENT_TEST = nullptr;

inline std::vector<BoltTest>& getTests() {
    static std::vector<BoltTest> tests;
    return tests;
}

inline void registerTest(BoltTest test) {
    getTests().push_back(test);
}

inline void logln(std::string msg,std::string& logs){
    logs += msg;
    logs += "\n";
}
inline void logln(std::string msg,std::ostream& stream){
    stream << msg;
    stream << "\n";
}
inline void log(std::string msg,std::string& logs){
    logs += msg;
}
inline void log(std::string msg,std::ostream& stream){
    stream << msg;
}

inline void logWithResult(std::string msg,BoltTestResult result,std::string& logs){
    if (COLORIZED_MODE) {
        logs += COLORIZE(msg,Utils::ColorUtils::getResultColor(result));
        logs += "\n";
    } else {
        logs += msg;
        logs += "\n";
    }
}
inline void logWithColor(std::string msg,Utils::ColorUtils::ANSIIColor color,std::string& logs){
    if (COLORIZED_MODE) {
        logs += COLORIZE(msg,color);
        logs += "\n";
    } else {
        logs += msg;
        logs += "\n";
    }
}
inline void logWithColorNL(std::string msg,Utils::ColorUtils::ANSIIColor color,std::string& logs){
    if (COLORIZED_MODE) {
        logs += COLORIZE(msg,color);
        logs += "\n";
    } else {
        logs += msg;
        logs += "\n";
    }
}
inline void logWithColor(std::string msg,Utils::ColorUtils::ANSIIColor color,std::ostream& stream){
    if (COLORIZED_MODE) {
        stream << COLORIZE(msg,color);
    } else {
        stream << msg;
    }
}
inline void logWithColorNL(std::string msg,Utils::ColorUtils::ANSIIColor color,std::ostream& stream){
    if (COLORIZED_MODE) {
        stream << COLORIZE(msg,color);
        stream << "\n";
    } else {
        stream << msg;
        stream << "\n";
    }
}

#define _str(args ...) Utils::StringUtils::as_str(args)

inline void runTests(std::ostream& out,std::ostream& timeProfiler = std::cout) {
    if(TIME_PROFILER_IS_ON){
        Utils::TimeUtils::Instrumentor::Get().BeginSession("BoltTest",timeProfiler);
    }
    BoltTestResult isCalculated;
    int fail = 0;
    int pass = 0;
    int sus_fail = 0;
    int sus_pass = 0;

    for (auto& test : getTests()) {
        test.flush();
        logln(_str("NAME:: " , test.name), out);
        logln(_str("DESC:: " , test.description), out);
        
        log(test.name, out);
        logWithColor(" test [START]",Utils::ColorUtils::BRIGHT_BLUE,out);
        log("\n", out);


        CURRENT_TEST = &test;
        if(TIME_PROFILER_IS_ON){
            PROFILE_SCOPE(test.name);
            isCalculated = test.test(test);
        }else{
            isCalculated = test.test(test);
        }
        pass = 0;
        fail = 0;
        sus_fail = 0;
        sus_pass = 0;

        for (auto& event : test.events) {
            std::string resultStr = std::string("[") + getResultName(event.result) + "] ";
            logWithColorNL(resultStr + event.message,Utils::ColorUtils::getResultColor(event.result), out);

            if (event.result == BoltTestResult::PASSED) pass++;
            else if (event.result == BoltTestResult::SUS_PASS) sus_pass++;
            else if (event.result == BoltTestResult::SUS_FAIL) sus_fail++;
            else fail++;
        }

        if (isCalculated == BoltTestResult::CALCULATED) {
            BoltTestResult result;
            float confidence = 0.0f;
            
            int score = (pass * 2 + sus_pass) - (fail * 2 + sus_fail);
            int totalWeight = pass * 2 + sus_pass + fail * 2 + sus_fail;
            
            if (totalWeight == 0) {
                confidence = 50.0f;
                result = BoltTestResult::SUS_PASS;
            } else {
                confidence = 50.0f + (100.0f * score) / (2.0f * totalWeight);
                confidence = std::max(0.0f, std::min(100.0f, confidence));
            
                if (confidence <= 20.0f) result = BoltTestResult::FAILED;
                else if (confidence <= 45.0f) result = BoltTestResult::SUS_FAIL;
                else if (confidence < 55.0f) result = BoltTestResult::SUS_PASS;
                else if (confidence < 80.0f) result = BoltTestResult::SUS_PASS;
                else result = BoltTestResult::PASSED;
            }

            std::string bar = "";
            for (int i = 0; i < 20; ++i) {
                if (i < (int)confidence / 5) bar += '=';
                else bar += ' ';
            }
            logWithColorNL("CONFIDENCE::[" + bar + "] " + std::to_string((int)confidence) + "%",Utils::ColorUtils::getResultColor(result),out);
            
            test.setResult(result);
            } 
        else {
            logWithColorNL("[WARNING] Test is not calculated, calculated mode is recommended for better debugging!",Utils::ColorUtils::BRIGHT_YELLOW,out);
            test.setResult(isCalculated);
        }

        std::string resultStr = getResultName(test.getResult());
        
        log(test.name,out);

        log(" pass:: ",out);
        logWithColor(std::to_string(pass),Utils::ColorUtils::GREEN,out);
        log(" X fail:: ",out);
        logWithColor(std::to_string(fail),Utils::ColorUtils::RED,out);
        log(" X sus_fail:: ",out);
        logWithColor(std::to_string(sus_fail),Utils::ColorUtils::BRIGHT_YELLOW,out);
        log(" X sus_pass:: ",out);
        logWithColor(std::to_string(sus_pass),Utils::ColorUtils::BRIGHT_BLUE,out);
        log(" = RESULT:: ",out);
        logWithColor(resultStr,Utils::ColorUtils::getResultColor(test.getResult()),out);

        

        log("\n",out);
        for(auto& context : test.contexts){
            logln(context,out);
        }
        log(test.name, out);
        logWithColor(" test [END]",Utils::ColorUtils::BRIGHT_RED,out);

        CURRENT_TEST = nullptr;
    }
    if(TIME_PROFILER_IS_ON){
        Utils::TimeUtils::Instrumentor::Get().EndSession();
    }

}

#define BOLT_TEST(NAME, DESCRIPTION, FUNC) \
    BoltTest NAME; \
    NAME.name = #NAME; \
    NAME.description = #DESCRIPTION; \
    NAME.test = FUNC; \
    registerTest(NAME);



    template<typename T>
    std::string to_string_helper(const T& value) {
        if constexpr (std::is_arithmetic_v<T>) {
            return std::to_string(value);
        } else if constexpr (std::is_pointer_v<T>) {
            if (value == nullptr) return "nullptr";
            std::ostringstream oss;
            oss << value;
            return oss.str();
        } else {
            std::ostringstream oss;
            oss << value;
            return oss.str();
        }
    }

inline std::string to_string_helper(const std::string& value) {
    return value;
}

inline std::string to_string_helper(const char* value) {
    return value ? std::string(value) : "nullptr";
}

#define BOLT_ASSERT_IMPL(OP, A, B, FILE, LINE) \
    do { \
        auto a_val = (A); \
        auto b_val = (B); \
        bool result = (a_val) OP (b_val); \
        BoltTestEvent event; \
        event.result = result ? BoltTestResult::PASSED : BoltTestResult::FAILED; \
        event.message = std::string{#A} + " " #OP " " + std::string{#B} + \
            " (" + to_string_helper(a_val) + " ?= " + to_string_helper(b_val) + ")" + \
            " at " + std::string{FILE} + ":" + std::to_string(LINE); \
        CURRENT_TEST->addEvent(event); \
    } while(false)

#define ADD_CONTEXT(CONTEXT) do { \
    CURRENT_TEST->contexts.push_back(CONTEXT); \
} while(false)

#define ASSERT_EQ(A, B) BOLT_ASSERT_IMPL(==, A, B, __FILE__, __LINE__)
#define ASSERT_NE(A, B) BOLT_ASSERT_IMPL(!=, A, B, __FILE__, __LINE__)
#define ASSERT_LT(A, B) BOLT_ASSERT_IMPL(<, A, B, __FILE__, __LINE__)
#define ASSERT_GT(A, B) BOLT_ASSERT_IMPL(>, A, B, __FILE__, __LINE__)
#define ASSERT_TRUE(A) BOLT_ASSERT_IMPL(==, A, true, __FILE__, __LINE__)
#define ASSERT_FALSE(A) BOLT_ASSERT_IMPL(==, A, false, __FILE__, __LINE__)
#define ASSERT_NULL(A) BOLT_ASSERT_IMPL(==, A, nullptr, __FILE__, __LINE__)
#define ASSERT_NOT_NULL(A) BOLT_ASSERT_IMPL(!=, A, nullptr, __FILE__, __LINE__)

#endif // BOLT_TEST_H



