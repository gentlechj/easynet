#pragma once

#include <string>
#include <time.h>

namespace easynet {
#define TimeStamp int64_t

std::string readableTime(time_t t);

TimeStamp now();
TimeStamp nowMilliseconds();
TimeStamp nowMicroseconds();

}