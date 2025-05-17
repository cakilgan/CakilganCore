#ifndef _COLOR_UTILS_H
#define _COLOR_UTILS_H

#include "logic_utils.h"
#include <string_utils.h>
#include <string>

namespace Utils::ColorUtils {
    struct ANSIIColor: public Utils::LogicUtils::canStringfy{
        int colorCode;
        ANSIIColor(int colorCode): colorCode(colorCode){}
        std::string toString() const override{
            return "\033[" + std::to_string(colorCode) + "m";
        }
    };
    static const ANSIIColor RESET{0};
    static const ANSIIColor BLACK{30};
    static const ANSIIColor RED{31};
    static const ANSIIColor GREEN{32};
    static const ANSIIColor YELLOW{33};
    static const ANSIIColor BLUE{34};
    static const ANSIIColor MAGENTA{35};
    static const ANSIIColor CYAN{36};
    static const ANSIIColor WHITE{37};
    static const ANSIIColor BRIGHT_BLACK{90};
    static const ANSIIColor BRIGHT_RED{91};
    static const ANSIIColor BRIGHT_GREEN{92};
    static const ANSIIColor BRIGHT_YELLOW{93};
    static const ANSIIColor BRIGHT_BLUE{94};
    static const ANSIIColor BRIGHT_MAGENTA{95};
    static const ANSIIColor BRIGHT_CYAN{96};
    static const ANSIIColor BRIGHT_WHITE{97};
}

#define COLORIZE(STR,COLOR) SURROUND(STR,COLOR.toString(),Utils::ColorUtils::RESET.toString())
#endif /* _COLOR_UTILS_H */
