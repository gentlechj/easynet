#pragma once

#include <time.h>

#include <string>

namespace easynet {
struct TimeStamp {
  TimeStamp(int64_t time) : m_time(time){};
  TimeStamp() = default;
  ~TimeStamp() = default;
  int64_t toSeconds() { return m_time / 1000; }
  static TimeStamp now();
  int64_t data() const { return m_time; }
  operator int64_t() const { return m_time; }
  const TimeStamp& operator+=(const TimeStamp& other) {
    this->m_time += other.data();
    return *this;
  }

 private:
  int64_t m_time;  // int64_t type form for milliseconds
};

inline const TimeStamp operator+(const TimeStamp& lhs, const TimeStamp& rhs) {
  return lhs.data() + rhs.data();
}

std::string readableTime(time_t t);
std::string readableTime(TimeStamp t);

int64_t now();

typedef struct TimeStamp TimeStamp;

}  // namespace easynet