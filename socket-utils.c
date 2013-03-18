#include "socket-utils.h"
#include "cache.h"
#include "run-command.h"
#ifndef WIN32
#else
#include <io.h>
#endif
int
is_socket(int fd)
{
	int result;
	const static char * func_name = "is_socket"; 
	struct stat st;
	if (fstat(fd, &st) < 0) {
		warning("failed to stat in %s: %s",
				func_name, strerror(errno));
		result = 0;
	}
	else {
		result = (st.st_mode & S_IFSOCK) == S_IFSOCK;
	}
	return result;
}
#else
int
is_socket(int fd)
{
	int st;
	int result;
	int optlen;
	optlen = sizeof(st);
	if (getsockopt(fd_to_socket(fd), SOL_SOCKET, SO_TYPE, (char *)&st, &optlen)) {
		result = 0;	
	} else {
		result = WSAGetLastError() == 0;
	}
	return result;
}

#endif
#ifdef EMULATE_TIME_WAIT_SOCKET
void set_socket_to_time_wait(int fd, int fd_is_out)
{
	struct linger lg;
	int state;
	int do_time_wait;
	int size;
	size = sizeof(lg);
	do_time_wait = 1;
	state = getsockopt(fd_to_socket(fd), 
		SOL_SOCKET, SO_LINGER, (char *)&lg, &size);
	if (state == 0)
	{
		do_time_wait = lg.l_onoff == 0;
	}
	if (do_time_wait)
	{
		struct child_process cmd;
		const char *argv[] = {
			"close-socket",
			NULL
		};
		memset(&cmd, 0, sizeof(cmd));
		if (fd_is_out)
		{
			cmd.out = fd;	
		}
		else
		{
			cmd.in = fd;
		}
		cmd.git_cmd = 1;	
		cmd.argv = argv;
		start_command(&cmd);
	}
}

#else
void set_socket_to_time_wait(int fd)
{
}
#endif

