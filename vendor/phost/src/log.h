/*
  Copyright (C) 2009-2013 NEC Corporation

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

extern uint8_t log_level;

enum log_levels {
    LOG_EMER = 0,
    LOG_ERR,
    LOG_WARN,
    LOG_DEBUG,
    LOG_LEVEL_MAX
};

#define LOG_LABEL_MAX 8

#define LOG_OUT_STDERR "stderr"
#define LOG_OUT_STDOUT "stdout"
#define LOG_OUT_FILE   "/var/log/phost.log"

#define log_emer(...)  log_level >= LOG_EMER ? \
                       log_output(LOG_EMER, __FILE__, __LINE__, \
                                 __FUNCTION__, __VA_ARGS__) : log_level;
#define log_err(...)   log_level >= LOG_ERR ? \
                       log_output(LOG_ERR, __FILE__, __LINE__, \
                                 __FUNCTION__, __VA_ARGS__) : log_level;
#define log_warn(...)  log_level >= LOG_WARN ? \
                       log_output(LOG_WARN, __FILE__, __LINE__, \
                                 __FUNCTION__, __VA_ARGS__) : log_level;
#define log_debug(...) log_level >= LOG_DEBUG ? \
                       log_output(LOG_DEBUG, __FILE__, __LINE__, \
                                 __FUNCTION__, __VA_ARGS__) : log_level;

int log_init(uint8_t level, const char *output);
int log_close();
int log_set_level(uint8_t level);
int log_set_output(const char *output);
uint8_t log_get_level();
int log_output(uint8_t level, const char *file, const int line,
               const char *function, char *format, ...);
char *log_get_lavel(uint8_t level);

#endif /* _LOG_H_ */
