#ifndef _STRING_UTILS_H
#define _STRING_UTILS_H
#include <logic_utils.h>
#include <string>
#include <sstream>
#include <vector>
namespace Utils::StringUtils{
    template<typename... Args>
    std::string as_str(Args&&... args) {
        std::ostringstream oss;
        (oss << ... << args);
        return oss.str();
    }
    template<typename T>
    std::string vec_to_str(const std::vector<T>& vec, const std::string& separator = " ") {
        std::ostringstream oss;
        
        if constexpr (std::is_base_of<Utils::LogicUtils::canStringfy, T>::value) {
            for (const auto& e : vec) {
                oss << e.toString() << separator;
            }
        }
        else {
            for (const auto& e : vec) {
                oss << e << separator;
            }
        }
    
        return oss.str();
    }
    inline void surround(std::string& str, const std::string& begin, const std::string& end) {
        str = begin + str + end;
    }
    inline void trim(std::string& trimit){
        for(int i=0;i<trimit.size();i++){
            if(trimit[i]==' '){
                trimit.erase(trimit.begin()+i);
                i--;
            }
        }
    }
}
#define SURROUND(str, prefix, suffix) std::string(prefix) + str + std::string(suffix)
#endif /* _STRING_UTILS_H */