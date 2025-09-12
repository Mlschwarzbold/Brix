#include <ctime>
#include <iostream>
#include "date_time_utils.h"

char *getCurrentDateString() {
    std::time_t t = std::time(0);   // Get the time now
    std::tm* now = std::localtime(&t);
    // return string in format YYYY-MM-DD
    static char date_str[11];
    std::strftime(date_str, sizeof(date_str), "%Y-%m-%d", now);
    return date_str;
}

char *getCurrentTimeString() {
    std::time_t t = std::time(0);   // Get the time now
    std::tm* now = std::localtime(&t);
    // return string in format HH:MM:SS
    static char time_str[9];
    std::strftime(time_str, sizeof(time_str), "%H:%M:%S", now);
    return time_str;
}