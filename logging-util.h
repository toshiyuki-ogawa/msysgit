#ifndef LOGGING_UTIL
#define LOGGING_UTIL
#include <stdarg.h>
extern int logging_is_verbose(void);
extern void logging_set_verbose(int new_verbose);
extern int logging_printf(const char *format, ...);
extern int logging_vprintf(const char *format, va_list va);
#endif
