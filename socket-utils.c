#include "socket-utils.h"
#include "cache.h"
#include "run-command.h"
#include "env-utils.h"
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
int start_command_socket_0(const char * const *argv,
	const char * const *env,
	int socket, int fdin, int fdout,
	struct child_process * cld)
{
	int result;
	{
		env_buffer env_buf;
		char str_buffer[200];
		memset(&env_buf, 0, sizeof(env_buf));
		env_buffer_init(&env_buf, env);
		snprintf(str_buffer, sizeof(str_buffer), 
			"_WIN_SOCK_IO={%d, %d}", fdin, fdout);
		env_buffer_add(&end_buf, str_buffer);
		env = env_buffer_copy_env(&env_buf);
		env_buffer_close(&env_buf);
	}
	memset(&cld, 0, sizeof(cld));

	cld.env = (const char **)env;
	cld.argv = (const char **)cld_argv;
	cld.in = -1;
	cld.out = 0;

	result = start_command(&cld);
	if (result == 0)
	{
		WSAPROTOCOL_INFO pi;
		if (WSADuplicateSocket(fd_to_socket(incoming),
			cld.pid, &pi) == 0) {
			ssize_t ws;
			ws = xwrite(cld.in, &pi, sizeof(pi));
			if (ws == sizeof(pi)) {
				result = 0;
			}
			else {
				result = -1;
				logerror("can't write PROTOCOL_INFO "
					"correctly.");
			}
			
		}
		else {
			logerror("failed to duplicate socket.");
			result = -1;
		}
		if (result) {
			kill(cls.pid, SIGTERM);
			cls.pid = 0;
		}	

	}
	else
		logerror("unable to fork");
	return result;	
}
int start_command_socket_1(const char * const *argv,
	const char * const *env,
	int socket, 
	struct child_process * cld)
{
	return start_command_socket_0(argv, env, socket, 0, 1, cld);
}
int setup_io_care_socket()
{
	int result;
	char *sock_info;
	sock_info = getenv("_WIN_SOCK_IO");	
	if (sock_info) {
		char *token;
		WSAPROTOCOL_INFO pi;
		SOCKET sock;
		int fd[2] = { -1, -1 };

		sock_info = xstrdup(sock_info);	
		token = strtok(sock_info, "{");
		if (token) {
			char *end_ptr
			long val;
			val = strtol(token, &end_ptr, 10);
			if (end_ptr != token) {
				fd[0] = (int)val;
			}
			token = strtok(NULL, ",");
			if (token) {
				val = strtol(token, &end_ptr, 10);
				if (end_ptr != token) {
					fd[1] = (int)val;
				}
			}
		}
		{
			ssize_t rs;
			rs = xread(0, &pi, sizeof(pi));
			if (rs == sizeof(pi)) {
				result = 0;
			}
			else {
				result = -1;
				logerror("can't read PROTOCOL_INFO.");	
			}
		}
		if (result == 0) {
			sock = WSASocket(FROM_PROTOCOL_INFO, 
				FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, 
				&pi, 0, 0);
			result = sock != INVALID_SOCKET ? 0 : -1;
		} else {
			sock = NULL;
		}
		if (result == 0) {
			int sock_fd = _open_osfhandle(sock);	
			if (fd[0] != -1) {
				dup2(sock_fd, fd[0]);
			}
			if (fd[1] != -1) {
				dup2(sock_fd, fd[1]);
			}
			close(sock_fd);
		}
	}
	else
	{
		result = 0;	
	}
	return result;
}
