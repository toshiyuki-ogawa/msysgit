#include "cache.h"
#include <time.h>
#include "socket-utils.h"

static int wait_for_time_out(int fd0, int fd1);
static time_t get_wait_time(int fd0, int fd1);

static time_t get_wait_time(int fd0, int fd1)
{
	return 60 * 1;
}
static int wait_for_time_out(int fd0, int fd1)
{
	time_t start_time;
	time_t wait_time;
	start_time = time(NULL);
	wait_time = get_wait_time(fd0, fd1);
	while (time(NULL) - start_time < wait_time) {
		sleep(1);	
	}
	return 0;
}

int main(int argc, char** argv)
{
	int result;
	
	close(2);
	result = 0;
	result = wait_for_time_out(0, 1);
	return result;
}
