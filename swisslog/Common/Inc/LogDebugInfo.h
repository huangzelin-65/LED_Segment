#ifndef  LOGDEBUGINFO_H
#define  LOGDEBUGINFO_H

#define LOG_LENGTH 500
#define DEBUGINFO_ALL(fmt, ...) safe_printf_all("[%s] " fmt"\n", __func__, ##__VA_ARGS__)

void safe_printf_all(const char *format, ...);

#endif
