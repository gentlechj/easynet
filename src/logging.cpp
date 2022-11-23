#include "logging.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <thread>
#include <stdarg.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/syslog.h>

#include "util.h"
// aaaaa
using namespace std;

namespace easynet
{
    Logger::Logger() : m_level(INFO)
    {
        tzset();
        m_fd = -1;
    }

    Logger::~Logger() noexcept
    {
        if (m_fd != -1)
        {
            close(m_fd);
        }
    }

    const char *Logger::levelStrs[ALL + 1] = {
        "FATAL",
        "ERROR",
        "WARNING",
        "INFO",
        "TRACE",
        "ALL",
    };

    Logger &Logger::getLogger()
    {
        static Logger logger;
        return logger;
    }

    void Logger::setLogLevel(const string &level)
    {
        LogLevel ilevel = INFO;
        for (size_t i = 0; i < sizeof(levelStrs) / sizeof(const char *); i++)
        {
            if (strcasecmp(levelStrs[i], level.c_str()) == 0)
            {
                ilevel = (LogLevel)i;
                break;
            }
        }
        setLogLevel(ilevel);
    }

    void Logger::setFileName(const std::string &filename)
    {
        int fd = open(filename.c_str(), O_APPEND | O_CREAT | O_WRONLY | O_CLOEXEC, DEFFILEMODE);
        if (fd < 0)
        {
            fprintf(stderr, "open log file %s failed. msg: %s ignored\n", filename.c_str(), strerror(errno));
            return;
        }
        m_filename = filename;
        if (m_fd == -1)
        {
            m_fd = fd;
        }
        else
        {
            int r = dup2(fd, m_fd);
            fatalif(r < 0, "dup2 failed");
            close(fd);
        }
    }

    static thread_local uint64_t tid;

    void Logger::logv(int level, const char *file, int line, const char *func, const char *fmt...)
    {
        if (tid == 0)
        {
            tid = gettid();
        }
        if (level > m_level)
        {
            return;
        }
        char buffer[4 * 1024];
        char *p = buffer;
        char *limit = buffer + sizeof(buffer);

        struct timeval now_tv;
        gettimeofday(&now_tv, NULL);
        const time_t seconds = now_tv.tv_sec;
        struct tm t;
        localtime_r(&seconds, &t);
        p += snprintf(p, limit - p, "%04d/%02d/%02d-%02d:%02d:%02d.%06d %lx %s %s:%d ", t.tm_year + 1900, t.tm_mon + 1,
                      t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,
                      static_cast<int>(now_tv.tv_usec), (long)tid, levelStrs[level], file, line);
        va_list args;
        va_start(args, fmt);
        p += vsnprintf(p, limit - p, fmt, args);
        va_end(args);
        p = std::min(p, limit - 2);
        // trim the ending \n
        while (*--p == '\n')
        {
        }
        *++p = '\n';
        *++p = '\0';

        int fd = m_fd == -1 ? 1 : m_fd;
        int err = ::write(fd, buffer, p - buffer);
        if (err != p - buffer)
        {
            fprintf(stderr, "write log file %s failed. written %d errmsg: %s\n", m_filename.c_str(), err, strerror(errno));
        }
        if (level <= ERROR)
        {
            syslog(LOG_ERR, "%s", buffer + 27);
        }
        if (level == FATAL)
        {
            fprintf(stderr, "%s", buffer);
            assert(0);
        }
    }
}