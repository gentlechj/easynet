#pragma once

#include <time.h>

#include <string>

namespace easynet {
// int64_t type form for milliseconds
#define TimeStamp int64_t

std::string readableTime(time_t t);
std::string readableTime(TimeStamp t);

TimeStamp now();
TimeStamp nowMilliseconds();
TimeStamp nowMicroseconds();

}  // namespace easynet