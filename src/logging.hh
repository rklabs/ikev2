/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
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

#ifndef _IKEV2_SRC_LOGGING_H_
#define _IKEV2_SRC_LOGGING_H_

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

#ifdef IKEV2_DEBUG
    #define TRACEF(level, msg)                  \
            FILE *fp = fopen("logfile" , "a+"); \
            fprintf(fp, "%s : %s:%s : "         \
                        "%s : %d : "            \
                        "%d : %s\n",            \
                        __FILE__, __DATE__,     \
                        __TIME__,               \
                        __PRETTY_FUNCTION__,    \
                        __LINE__, level, msg);  \
            fclose(fp); \

    #define TRACEO(level, msg)                      \
            fprintf(stdout, "%s : %s:%s : "         \
                            "%s : %d : "            \
                            "%d : %s\n",            \
                            __FILE__, __DATE__,     \
                            __TIME__,               \
                            __PRETTY_FUNCTION__,    \
                            __LINE__, level, msg);  \

    #define LOG(level, ...) \
        ikev2::logging::logger::getLogger().log(level, __VA_ARGS__)

    #define TRACE() \
        LOG(ikev2::logging::INFO, "%s : %s : %d", __FILE__, \
                                                  __PRETTY_FUNCTION__, \
                                                  __LINE__);
#else
    #define TRACEF(level, msg) (void) 0
    #define TRACEO(level, msg) (void) 0
    #define LOG(level, ...) (void) 0
    #define TRACE() (void) 0
#endif

#define IKEV2_LOGOBJ_NAME "ikev2logger"
#define IKEV2_LOG_FILENAME "ikev2.log"

namespace ikev2 {
namespace logging {

    enum LogLevel {
        EMERG,   // System is unusable (e.g. multiple parts down)
        ALERT,   // System is unusable (e.g. single part down)
        CRIT,    // Failure in non-primary system (e.g. backup site down)
        ERROR,   // Non-urgent failures; relay to developers
        WARN,    // Not an error, but error will occurr if nothing done.
        NOTICE,  // Events that are unusual, but not error conditions.
        INFO,    // Normal operational messages. No action required.
        DEBUG,   // Information useful during development for debugging.
        NOTSET
    };

    class logger final {
     public:
        // Static function to get only one instance of logger
        static logger& getLogger();
        void log(LogLevel level, const char *fmt, ...);
        void enableAllLogLevels();
        void enableLogLevel(LogLevel level);

     private:
        logger();
        ~logger();
        logger(const logger&) =delete;
        logger(logger&&) =delete;
        logger& operator=(const logger&) =delete;
        logger& operator=(logger&&) =delete;

        log4cpp::Category& loggerCategory;
        log4cpp::RollingFileAppender *fileAppender;
        log4cpp::PatternLayout *loggerlayout;
        int levelsEnabled;
    };

}  // namespace logging
}  // namespace ikev2

// Short form to be using in while logging
namespace LOG = ikev2::logging;

#endif  // _IKEV2_SRC_LOGGING_H_

