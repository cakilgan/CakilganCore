#ifndef _MATH_UTILS_H
#define _MATH_UTILS_H
#include <numbers>
namespace Utils::MathUtils{
  
    inline float degtorad(float deg){
        float ret = deg * std::numbers::pi / 180.0f;
        return ret;
    }
    
    inline float radtodeg(float rad){
        float ret = rad * 180.0f / std::numbers::pi;
        return ret;
    }    
}

#endif /* _MATH_UTILS_H */

