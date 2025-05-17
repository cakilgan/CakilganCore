#ifndef _LOGIC_UTILS_H
#define _LOGIC_UTILS_H
#include <string>
namespace Utils::LogicUtils{
    struct canStringfy{
        virtual std::string toString() const = 0;
    };
}

#endif /* _LOGIC_UTILS_H */

