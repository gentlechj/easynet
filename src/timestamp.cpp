#include "timestamp.h"

#include <chrono>

#include "util.h"

namespace easynet {

std::string readableTime(time_t t) {
  struct tm tm1;
  localtime_r(&t, &tm1);
  return format("%04d-%02d-%02d %02d:%02d:%02d", tm1.tm_year + 1900,
                tm1.tm_mon + 1, tm1.tm_mday, tm1.tm_hour, tm1.tm_min,
                tm1.tm_sec);
}

std::string readableTime(TimeStamp t) {
  t = t / 1000;
  return readableTime(time_t(t));
}

TimeStamp now() { return nowMilliseconds(); }
TimeStamp nowMilliseconds() {
  std::chrono::time_point<std::chrono::system_clock> p =
      std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             p.time_since_epoch())
      .count();
}
TimeStamp nowMicroseconds() {
  std::chrono::time_point<std::chrono::system_clock> p =
      std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(
             p.time_since_epoch())
      .count();
}

}  // namespace easynet