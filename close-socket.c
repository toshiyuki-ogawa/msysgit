#include "cache.h"
#include <time.h>
#include "winsock-utils.h"

const static int DEFAULT_TIME_WAIT = 240;

static int wait_for_time_out(void)
{
	time_t start_time;
	time_t wait_time;
	start_time = time(NULL);
	wait_time = get_socket_time_wait();
	while (time(NULL) - start_time < wait_time) {
		sleep(1);	
	}
	return 0;
}

int main(int argc, char** argv)
{
	int result;
	
	result = 0;
	result = wait_for_time_out();
	return result;
}
