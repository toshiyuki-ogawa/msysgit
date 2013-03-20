#include "logging-util.h"
#include <stdio.h>

int
logging_printf(const char *format, ...)
{
	int result;
	va_list va;
	va_start(va, format);
	result = logging_vprintf(format, va);
	va_end(va);
	return result;
}
int
logging_vprintf(const char *format, va_list va)
{
	int result;
	if (logging_is_verbose())
	{
		result = vfprintf(stderr, format, va);
	}
	else
	{
		result = 0;
	}
	return result;
}

static int verbose_state;

void
logging_set_verbose(int new_verbose)
{
	verbose_state = new_verbose;
}

int
logging_is_verbose(void)
{

	return verbose_state;
}


