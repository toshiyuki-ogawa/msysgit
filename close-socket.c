#include "cache.h"
#include <time.h>
#include "socket-utils.h"

const static int DEFAULT_TIME_WAIT = 240;

static int wait_for_time_out(int fd0, int fd1);
static time_t get_wait_time(int fd0, int fd1);

static int read_reg_time_wait(time_t *time_wait)

static int read_reg_time_wait(time_t *time_wait)
{
	HKEY hk;
	LONG state;
	int result;
	hk = NULL;
	state = RegOpenKeyExW(HKEY_LOCAL_MATCHINE,
			L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\"
			L"Parameters",
			NULL,
			KEY_READ,
			&hk);
	if (state == ERROR_SUCCESS) {
		DWORD data_type;
		DWORD value;
		DWORD value_size;
		value_size = sizeof(value);
		state = RegQueryValueExW(hk, L"TcpTimedWaitDelay", NULL, 
			&data_type, &value, &value_size);	
		if (state == ERROR_SUCCESS && data_type == REG_DWORD) {
			*time_wait = (time_t)value;
			result = 1;
		}
		else {
			result = 0;
		}
		RegCloseKey(hk);
	} else {
		result = 0;
	}
	return result;
}

static int read_time_wait_from_registry(time_t *time_wait)
{
	int result;
	result = read_reg_time_wait(time_wait);
	if (result)
	{
		*time_wait /= 2;
	}
	return result;
}

static int read_time_wait_from_config(time_t *time_wait)
{
	int result;
	long value;
	result = git_parse_long("win.sock.time.wait", &value);
	if (result) {
		*time_wait = (time_t)value;
	}
	return result;
}

static time_t get_wait_time(int fd0, int fd1)
{

	int state;
	long value;
	time_t result;
	int i;
	int (*read_time_wait[])(time_t *) = {
		read_time_wait_from_config,
		read_time_wait_from_registry
	};
	for (i = 0; i < sizeof(read_time_wait) / sizeof(read_time_wait[0]); 
		i++) {
		if (read_time_wait[i](&result)) {
			break;
		}
	}
	if (i == sizeof(read_time_wait) / sizeof(read_time_wait[0])) {
		result = (time_t)DEFAULT_TIME_WAIT;
	}
	return result;
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
	
	result = 0;
	result = wait_for_time_out(0, 1);
	return result;
}
