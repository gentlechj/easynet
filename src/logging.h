#pragma once

#include <string>

#include "util.h"

#ifdef NDEBUG
#define log(level, ...)                                             \
  do {                                                              \
    if (level <= Logger::getLogger().getLogLevel()) {               \
      Logger::getLogger().logv(level, __FILE__, __LINE__, __func__, \
                               __VA_ARGS__);                        \
    }                                                               \
  } while (0)
#else
#define log(level, ...)                                             \
  do {                                                              \
    if (level <= Logger::getLogger().getLogLevel()) {               \
      snprintf(0, 0, __VA_ARGS__);                                  \
      Logger::getLogger().logv(level, __FILE__, __LINE__, __func__, \
                               __VA_ARGS__);                        \
    }                                                               \
  } while (0)
#endif

#define trace(...) log(Logger::TRACE, __VA_ARGS__)
#define info(...) log(Logger::INFO, __VA_ARGS__)
#define warn(...) log(Logger::WARNING, __VA_ARGS__)
#define error(...) log(Logger::ERROR, __VA_ARGS__)
#define fatal(...) log(Logger::FATAL, __VA_ARGS__)

#define fatalif(b, ...)                \
  do {                                 \
    if ((b)) {                         \
      log(Logger::FATAL, __VA_ARGS__); \
    }                                  \
  } while (0)

#define setloglevel(l) Logger::getLogger().setLogLevel(l)
#define setlogfile(n) Logger::getLogger().setFileName(n)

namespace easynet {
class Logger : private noncopyable {
 public:
  enum LogLevel { FATAL = 0, ERROR, WARNING, INFO, TRACE, ALL };

  Logger();

  ~Logger() noexcept;

  void logv(int level, const char *file, int line, const char *func,
            const char *fmt...);

  void setFileName(const std::string &filename);

  void setLogLevel(const std::string &level);

  void setLogLevel(LogLevel level) {
    m_level = std::min(ALL, std::max(FATAL, level));
  }

  LogLevel getLogLevel() { return m_level; }

  const char *getLogLevelStr() { return levelStrs[m_level]; }

  int getFd() { return m_fd; }

  void adjustLogLevel(int adjust) { setLogLevel(LogLevel(m_level + adjust)); }

  static Logger &getLogger();

 private:
  static const char *levelStrs[ALL + 1];

  int m_fd;
  LogLevel m_level;
  std::string m_filename;
};
}  // namespace easynet
