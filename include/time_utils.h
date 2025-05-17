#ifndef TIME_UTILS_H
#define TIME_UTILS_H
#include <chrono>
#include <string>
#include <thread>
#include <iostream>
#include <mutex>
inline int GLOBAL_DEBUG_N = -1;
#define _PROFILE_SCOPE(name, debugn) \
    if (GLOBAL_DEBUG_N == debugn) Utils::TimeUtils::ScopeTimer timer##__LINE__(name)

#define PROFILE_SCOPE(name) Utils::TimeUtils::ScopeTimer timer##__LINE__(name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__)
#define SCOPE(name) \
    if (int _scope_flag = 0) {} else for (Utils::TimeUtils::ScopeTimer timer__LINE__(name); !_scope_flag; _scope_flag = 1)

#define TEST_CASE(name) \
    void name(); \
    struct name##_registrar { \
        name##_registrar() { registerTest(name, #name); } \
    } name##_registrar_instance; \
    void name() { \
        PROFILE_SCOPE(#name);

namespace Utils::TimeUtils {
    struct ProfileResult {
        std::string name;
        long long start, end;
        uint32_t threadID;
    };
    
    class Instrumentor {
        public:
            static Instrumentor& Get() {
                static Instrumentor instance;
                return instance;
            }
        
            void BeginSession(const std::string& name, std::ostream& out) {
                std::lock_guard<std::mutex> lock(mutex_);
                output = &out;
                *output << "{\"otherData\": {},\"traceEvents\":[";
                sessionActive = true;
            }
        
            void EndSession() {
                std::lock_guard<std::mutex> lock(mutex_);
                if (!sessionActive || !output) return;
                *output << "{}]}";
                output = nullptr;
                sessionActive = false;
            }
        
            void WriteProfile(const ProfileResult& result) {
                std::lock_guard<std::mutex> lock(mutex_);
                if (!sessionActive || !output){
                    return;
                } 
                *output << "{";
                *output << "\"cat\":\"function\",";
                *output << "\"dur\":" << (result.end - result.start) << ',';
                *output << "\"name\":\"" << result.name << "\",";
                *output << "\"ph\":\"X\",";
                *output << "\"pid\":0,";
                *output << "\"tid\":" << result.threadID << ",";
                *output << "\"ts\":" << result.start;
                *output << "},";
            }
        inline bool isActive(){
            return sessionActive;
        }
        private:
            std::ostream* output = nullptr;
            std::mutex mutex_;
            bool sessionActive = false;
        };
    
    class ScopeTimer {
    public:
        ScopeTimer(const std::string& name)
            : name(name), start(std::chrono::high_resolution_clock::now()) {}
    
        ~ScopeTimer() {
            auto endTime = std::chrono::high_resolution_clock::now();
            long long startNs = std::chrono::time_point_cast<std::chrono::microseconds>(start).time_since_epoch().count();
            long long endNs = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();
            uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
    
            Instrumentor::Get().WriteProfile({ name, startNs, endNs, threadID });
        }
    
    private:
        std::string name;
        std::chrono::time_point<std::chrono::high_resolution_clock> start;
    };
    class DefaultTimer{
        private:
        std::string name;
        std::chrono::time_point<std::chrono::high_resolution_clock> _start;
        public:
        DefaultTimer(const std::string& name)
            : name(name){}
    
        void start(){
            _start = std::chrono::high_resolution_clock::now();
        }
        void defaultStop(){
            write(stop());
        }
        ProfileResult stop(){
            auto endTime = std::chrono::high_resolution_clock::now();
            long long startNs = std::chrono::time_point_cast<std::chrono::microseconds>(_start).time_since_epoch().count();
            long long endNs = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();
            uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
            return {name, startNs, endNs, threadID};
        }
        void write(ProfileResult result){
            Instrumentor::Get().WriteProfile(result);
        }
    
    };
}
#endif