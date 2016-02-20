/*
 * Copyright (C) 2014 Raju Kadam <rajulkadam@gmail.com>
 *
 * IKEv2 is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * IKEv2 is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <syslog.h>
#include <stdio.h>
#include <stdarg.h>

#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/RollingFileAppender.hh>

#include "basictypes.hh"

#ifdef IKEV2_DBG
    #define TRACEF(level, msg)                  \
            FILE *fp = fopen("logfile" , "a+"); \
            fprintf(fp, "%s : %s:%s : "         \
                        "%s : %d : "            \
                        "%d : %s\n",            \
                        __FILE__, __DATE__,     \
                        __TIME__,               \
                        __PRETTY_FUNCTION__,    \
                        __LINE__, level, msg);  \
            fclose(fp);                         \

    #define TRACE0(level, msg)                      \
            fprintf(stdout, "%s : %s:%s : "         \
                            "%s : %d : "            \
                            "%d : %s\n",            \
                            __FILE__, __DATE__,     \
                            __TIME__,               \
                            __PRETTY_FUNCTION__,    \
                            __LINE__, level, msg);  \

    #define LOG(level, ...) \
        Logging::Logger::getLogger().log(level, __VA_ARGS__) \

    #define TRACE() \
        LOG(INFO, "TRACE %s : %s : %d", __FILE__,            \
                                        __PRETTY_FUNCTION__, \
                                        __LINE__); \

    #define LOGT(fmt, ...) do { fprintf(stderr, fmt, ##__VA_ARGS__); \
                                fprintf(stderr, "\n");} while(0);
#else
    #define TRACEF(level, msg)  (void) 0
    #define TRACEO(level, msg)  (void) 0
    #define LOG(level, ...)     (void) 0
    #define TRACE()             (void) 0
    #define LOGT(msg, ...)      (void) 0
#endif

#define IKEV2_LOG_OBJ_NAME "ikev2Logger"
#define IKEV2_LOG_FILENAME "ikev2.log"

enum LogLevel {
    EMERG,   // 0 System is unusable (e.g. multiple parts down)
    ALERT,   // 1 System is unusable (e.g. single part down)
    CRIT,    // 2 Failure in non-primary system (e.g. backup site down)
    ERROR,   // 3 Non-urgent failures; relay to developers
    WARN,    // 4 Not an error, but error will occurr if nothing done.
    NOTICE,  // 5 Events that are unusual, but not error conditions.
    INFO,    // 6 Normal operational messages. No action required.
    DEBUG,   // 7 Information useful during development for debugging.
    NOTSET
};

// XXX future - make use of spdlog
// XXX https://github.com/gabime/spdlog
// XXX logging performance is #@#%U bad

namespace Logging {

class Logger final {
 public:
    Logger();
    ~Logger();
    // Static function to get only one instance of Logger
    static Logger & getLogger();
    void log(LogLevel level, const char * fmt, ...);
    void enableAllLogLevels();
    void enableLogLevel(LogLevel level);

 private:
    S32 levelsEnabled;
    log4cpp::Category & loggerCategory;
    log4cpp::RollingFileAppender * fileAppender;
    log4cpp::PatternLayout * loggerLayout;

    // Delete all constructors
    Logger(const Logger &)=delete;
    Logger(Logger &&)=delete;
    Logger & operator=(const Logger &)=delete;
    Logger & operator=(Logger &&)=delete;
};

}  // namespace Logging

// Short form to be using in while Logging
namespace LOG = Logging;
