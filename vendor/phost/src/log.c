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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/param.h>
#include "log.h"

uint8_t log_level;
static char *log_file = NULL;

static char log_lavels[LOG_LEVEL_MAX][LOG_LABEL_MAX] = {
    "EMER",
    "ERR",
    "WARN",
    "DEBUG"
};

static FILE *log_fp = NULL;

int log_init(uint8_t level, const char *output)
{
    log_close();
    return (log_set_level(level) | log_set_output(output));
}

int log_close()
{
    if(log_file != NULL){
        free(log_file);
        fclose(log_fp);
        log_file = NULL;
        log_fp = NULL;
    }

    return 0;
}

int log_set_level(uint8_t level)
{
    log_level = level;

    return 0;
}

int log_set_output(const char *output)
{
    log_close();

    if(strncmp(output, LOG_OUT_STDOUT, strlen(LOG_OUT_STDOUT)) == 0){
        log_fp = stdout;
    }
    else if(strncmp(output, LOG_OUT_STDERR, strlen(LOG_OUT_STDERR)) == 0){
        log_fp = stderr;
    }
    else{
        log_fp = fopen(output, "w");
        if(log_fp == NULL){
            return -1;
        }
        log_file = (char*)malloc(sizeof(char)*(strlen(output)+1));
        strcpy(log_file, output);
    }

    return 0;
}

uint8_t log_get_level()
{
    return log_level;
}

int log_output(uint8_t level, const char *file, const int line,
               const char *function, char *format, ...)
{
    if(level > log_level){
        return 0;
    }

    if(log_fp == NULL){
        return -1;
    }

    struct timeval tv;
    struct tm tm;

    gettimeofday(&tv, NULL);
    localtime_r(&(tv.tv_sec), &tm);

    fprintf(log_fp, "%04d/%02d/%02d %02d:%02d:%02d.%06d [%5s] (%s:%d:%s) ",
            tm.tm_year + 1900,
            tm.tm_mon + 1,
            tm.tm_mday,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec,
            (int)tv.tv_usec,
            log_get_lavel(level),
            file,
            line,
            function);

    va_list args;
    va_start(args, format);
    vfprintf(log_fp, format, args);
    va_end(args);
    fprintf(log_fp, "\n");

    fflush(log_fp);

    return 0;
}

char *log_get_lavel(uint8_t level){
    return log_lavels[level];
}
