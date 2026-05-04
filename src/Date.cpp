#include "Date.h"
#include <cstdio>

bool Date::operator==(const Date& other) const {
    return day == other.day && month == other.month && year == other.year;
}

bool Date::isBefore(const Date& other) const {
    if (year != other.year) return year < other.year;
    if (month != other.month) return month < other.month;
    return day < other.day;
}

bool Date::isAfter(const Date& other) const {
    return other.isBefore(*this);
}

bool Date::isBetween(const Date& start, const Date& end) const {
    return !isBefore(start) && !isAfter(end);
}

std::string Date::toString() const {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", year, month, day);
    return std::string(buf);
}
