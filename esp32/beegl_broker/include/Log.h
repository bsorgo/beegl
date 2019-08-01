/*
  Log.h - Log interface
  
  This file is part of the BeeGl distribution (https://github.com/bsorgo/beegl).
  Copyright (c) 2019 Bostjan Sorgo
  
  This program is free software: you can redistribute it and/or modify  
  it under the terms of the GNU General Public License as published by  
  the Free Software Foundation, version 3.
 
  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
  General Public License for more details.
 
  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef Log_h
#define Log_h


#include <FS.h>

#include <SPI.h>

#include <ArduinoNvs.h>

#include <AsyncJson.h>
#include <ArduinoJson.h>

#include "Storage.h"


#ifndef MAX_LOG_FILES
    #define MAX_LOG_FILES  10
#endif
#ifndef MAX_LOG_FILE_SIZE
    #define MAX_LOG_FILE_SIZE  4096
#endif

#define LOG_DIR  "/log"
#define LOG_DIR_PREFIX  LOG_DIR "/"
#define LOG_EXTENSION ".log"
#define LAST_LOG_NVS "last_log"

int fs_log_printf(const char *fmt, ...);
long log_number() ;
#define BEEGL_LONG_LOG_FORMAT(letter, format) "[" #letter "][%s:%u] %s(): " format  "\r\n", pathToFileName(__FILE__), __LINE__, __FUNCTION__
#define BEEGL_SHORT_LOG_FORMAT(letter, format)  format  "\r\n"

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_VERBOSE
#define blog_v(format, ...) do {\
    log_printf(ARDUHAL_LOG_FORMAT(V, format), ##__VA_ARGS__);\
    fs_log_printf(BEEGL_SHORT_LOG_FORMAT(V,format), ##__VA_ARGS__);\
} while (0)
 
#else
#define blog_v(format, ...)
#endif

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
#define blog_d(format, ...) do {\
    log_printf(ARDUHAL_LOG_FORMAT(D, format), ##__VA_ARGS__);\
    fs_log_printf(BEEGL_SHORT_LOG_FORMAT(V,format), ##__VA_ARGS__);\
} while (0)
#else
#define blog_d(format, ...)
#endif

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
#define blog_i(format, ...) do {\
    log_printf(ARDUHAL_LOG_FORMAT(I, format), ##__VA_ARGS__);\
    fs_log_printf(BEEGL_SHORT_LOG_FORMAT(V,format), ##__VA_ARGS__);\
} while (0)
#else
#define blog_i(format, ...)
#endif

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_WARN
#define blog_w(format, ...) do {\
    log_printf(ARDUHAL_LOG_FORMAT(W, format), ##__VA_ARGS__);\
    fs_log_printf(BEEGL_SHORT_LOG_FORMAT(V,format), ##__VA_ARGS__);\
} while (0)
#else
#define blog_w(format, ...)
#endif

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_ERROR
#define blog_e(format, ...) do {\
    log_printf(ARDUHAL_LOG_FORMAT(E, format), ##__VA_ARGS__);\
    fs_log_printf(BEEGL_SHORT_LOG_FORMAT(V,format), ##__VA_ARGS__);\
} while (0)
#else
#define blog_e(format, ...)
#endif

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_NONE
#define blog_n(format, ...) log_printf(ARDUHAL_LOG_FORMAT(E, format), ##__VA_ARGS__)
#else
#define blog_n(format, ...)
#endif

#endif