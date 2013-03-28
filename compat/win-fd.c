#include "cache.h"
#include "win-fd.h"

#include <io.h>
#include <stdarg.h>
#include <stddef.h>

int
win_fd_status_count_fd(va_list args);

int
win_fd_status_arg_to_array(va_list args, int *size, int **fd_array);


win_fd_status*
win_fd_status_alloc(int size);

int
win_fd_status_count_fd(va_list args)
{
	va_list args_wk;
	int result;
	int val;
	result = 0;
	va_copy(args_wk, args);
	val = va_arg(args_wk, int);
	while (val >= 0)
	{
		result++;
		val = va_arg(args_wk, int);	
	}
	va_end(args_wk);
	return result;
}

win_fd_status*
win_fd_status_alloc(int size)
{
	win_fd_status *result;
	size_t status_size;
	size_t header_size;
	header_size = offsetof(win_fd_status, status);
	status_size = sizeof(win_fd_status) - header_size;

	result = (win_fd_status *)xmaloc(header_size + status_size * size);
	if (result)
	{
		result->size = size;
		memset(&result->status[0], 0, status_size * size);
	}
	return result;
}

int
win_fd_status_arg_to_array(va_list args, int *size, int **fd_array)
{
	int num_of_fd;
	va_list args_wk;
	int* fd_array_wk;
	int result;
	va_copy(args_wk, args);
	num_of_fd = win_fd_status_count_fd(args_wk);

	if (num_of_fd) {
		fd_array_wk = (int *)xmalloc(sizeof(int) * num_of_fd);
		if (fd_array_wk) {
			int i;
			for (i = 0; i < num_of_fd; i++) {
				fd_array_wk[i] = va_arg(args_wk, int);	
			}
			result = 0;
		}
		else {
			result = -1;
		}

	}
	else {
		fd_array_wk = NULL;
		result = 0;
	}
	if (result == 0)
	{
		*size = num_of_fd;
		*fd_array = fd_array_wk;
	}
	
	va_end(args_wk);

	return result;
}



