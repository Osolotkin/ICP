#pragma once

#include <stdio.h>
#include "log.h"

#define RED_ESC "\033[1;31m"
#define YELLOW_ESC "\033[1;33m"
#define WHITE_ESC "\037[1;37m"
#define COLOR_RESET_ESC "\033[0m"

namespace Log {

    void log(const Level type, const char* const message) {

        switch (type) {

            case INFO : {
                printf(WHITE_ESC "INFO : " COLOR_RESET_ESC);
                break;
            }

            case WARNING : {
                printf(YELLOW_ESC "WARNING : " COLOR_RESET_ESC);
                break;
            }

            case ERROR : {
                printf(RED_ESC "ERROR : " COLOR_RESET_ESC);
                break;
            }
        
            default :
                break;
        
        }

        printf(message);
        printf("\n");

    }

    void logError(const Err::Err error) {
        log(Log::ERROR, ERR_STR(error));
    }

}
