/*
  Log.cpp - runtime log
  
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

#include "Log.h"
namespace beegl
{
int fs_log_printf(const char *fmt, ...);
int lastLog = 0;
long lastLogFileSize = 0;
const long maxLogFileSize = MAX_LOG_FILE_SIZE;
const int maxLogFiles = MAX_LOG_FILES;

File logFile;

int fs_log_printf(const char *format, ...)
{

    static char loc_buf[64];
    char *temp = loc_buf;
    int len;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    len = vsnprintf(NULL, 0, format, arg);
    va_end(copy);
    if (len >= sizeof(loc_buf))
    {
        temp = (char *)malloc(len + 1);
        if (temp == NULL)
        {
            return 0;
        }
    }
    vsnprintf(temp, len + 1, format, arg);
    String filename;

    if (!logFile)
    {
#ifdef FILESYSTEM_SD
        uint8_t cardType = SD.cardType();
        if (cardType == CARD_NONE)
        {
            log_e("[LOG] No SD card attached.");
            return 0;
        }
#endif
        if (!FILESYSTEM.exists(LOG_DIR))
        {

            if (!FILESYSTEM.mkdir(LOG_DIR))
            {
                log_e("[LOG] Error creating dir: %s", LOG_DIR);
            }
        }
        lastLog = NVS.getInt(LAST_LOG_NVS);
        if (lastLog == 0)
        {
            do
            {
                lastLog++;
                filename = String(LOG_DIR_PREFIX);
                filename += String(lastLog);
                filename += LOG_EXTENSION;
            } while (FILESYSTEM.exists(filename));
            NVS.setInt(LAST_LOG_NVS, lastLog);
        }
        else
        {
            filename = String(LOG_DIR_PREFIX);
            filename += String(lastLog);
            filename += LOG_EXTENSION;
        }

        log_d("[LOG] Log filename: %s", filename.c_str());
        logFile = FILESYSTEM.open(filename, FILE_APPEND);
        if (!logFile)
        {
            log_e("[LOG] Failed to open log file to append: %s", filename.c_str());
            return 0;
        }
        lastLogFileSize = logFile.size();
    }

    if (lastLogFileSize + strlen(temp) > maxLogFileSize)
    {
        logFile.close();
        lastLog++;
        filename = String(LOG_DIR_PREFIX);
        filename += String(lastLog);
        filename += LOG_EXTENSION;
        logFile = FILESYSTEM.open(filename, FILE_APPEND);
        lastLogFileSize = 0;
        NVS.setInt(LAST_LOG_NVS, lastLog);
        log_d("[LOG] New log filename: %s", filename.c_str());
        if (!logFile)
        {
            log_e("[LOG] Failed to open new log file to append: %s", filename.c_str());
            return 0;
        }
        int removeLog = lastLog - maxLogFiles;
        if (removeLog > 0)
        {
            String removeFilename = String(LOG_DIR_PREFIX);
            removeFilename += String(removeLog);
            removeFilename += LOG_EXTENSION;
            if (!FILESYSTEM.remove(removeFilename))
            {
                log_e("[LOG] Failed to remove log: %s", removeFilename.c_str());
            }
        }
    }

    logFile.write((uint8_t *)temp, (size_t)len);
    lastLogFileSize += len;
    logFile.flush();
    va_end(arg);
    if (len >= sizeof(loc_buf))
    {
        free(temp);
    }
    return len;
}

long log_number()
{
    return lastLog;
}
} // namespace beegl
