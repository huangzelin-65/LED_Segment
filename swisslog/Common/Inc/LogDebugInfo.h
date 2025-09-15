#ifndef  LOGDEBUGINFO_H
#define  LOGDEBUGINFO_H

#define LOG_LENGTH 100

#define DEBUGINFO(fmt, ...) safe_printf("[%s] " fmt, __func__, ##__VA_ARGS__)
#define DEBUGINFO_ISR(fmt, ...) safe_printf_isr("[%s] " fmt, __func__, ##__VA_ARGS__)
#define DEBUGINFO_ALL(fmt, ...) safe_printf_all("[%s] " fmt"\n", __func__, ##__VA_ARGS__)

void safe_printf(const char *format, ...);
void safe_printf_isr(const char *format, ...);
void safe_printf_all(const char *format, ...);
#endif
