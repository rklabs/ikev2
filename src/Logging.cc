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

#include "Logging.hh"

namespace Logging {

Logger::Logger():
        loggerCategory(log4cpp::Category::getRoot()),
        fileAppender(new log4cpp::RollingFileAppender(IKEV2_LOG_OBJ_NAME,
                                                      IKEV2_LOG_FILENAME)),
        loggerLayout(new log4cpp::PatternLayout()) {
    // By default, set to "%m%n".
    // Format characters are as follows:
    // %%%% - a single percent sign
    // %c - the category
    // %d - the date
    // Date format: The date format character may be followed by a date
    // format specifier enclosed between braces. For example,
    // %d{%H:%M:%S,%l} or %d{%d %m %Y %H:%M:%S,%l}. If no date format
    // specifier is given then the following format is used:
    // "Wed Jan 02 02:03:55 1980". The date format specifier admits
    // the same syntax as the ANSI C function strftime, with 1 addition.
    // The addition is the specifier %l for milliseconds, padded with
    // zeros to make 3 digits.
    // %m - the message
    // %n - the platform specific line separator
    // %p - the priority
    // %r - milliseconds since this layout was created.
    // %R - seconds since Jan 1, 1970
    // %u - clock ticks since process start
    // %x - the NDC
    loggerLayout->setConversionPattern("%d{%Y-%m-%d %H:%M:%S:%l} [%p] %c: %m%n");

    fileAppender->setLayout(loggerLayout);

    loggerCategory.setPriority(log4cpp::Priority::DEBUG);
    loggerCategory.addAppender(fileAppender);
}

// Return global static logger(singleton)
Logger &
Logger::getLogger() {
    static Logger l;
    return l;
}

void
Logger::log(LogLevel level, const char * fmt, ...) {
    auto loglevel = log4cpp::Priority::NOTSET;
    va_list arg;

    switch (level) {
        case NOTSET:
        case EMERG:
            loglevel = log4cpp::Priority::EMERG;
            break;
        case ALERT:
            loglevel = log4cpp::Priority::ALERT;
            break;
        case CRIT:
            loglevel = log4cpp::Priority::CRIT;
            break;
        case ERROR:
            loglevel = log4cpp::Priority::ERROR;
            break;
        case WARN:
            loglevel = log4cpp::Priority::WARN;
            break;
        case NOTICE:
            loglevel = log4cpp::Priority::NOTICE;
            break;
        case INFO:
            loglevel = log4cpp::Priority::INFO;
            break;
        case DEBUG:
            loglevel = log4cpp::Priority::DEBUG;
            break;
        default:
            // Invalid log level
            assert(false);
    }

    // Invalid log level
    assert(loglevel != log4cpp::Priority::NOTSET);

    // Log the message and arguments
    va_start(arg, fmt);
    loggerCategory.logva(loglevel, fmt, arg);
    va_end(arg);
}

Logger::~Logger() {
    // XXX required ?
    fileAppender->close();
}

}  // namespace Logging
