#include "cache.h"
#include "socket-utils.h"
#include "run-command.h"
#include "env-utils.h"
#include "logging-util.h"
#ifndef WIN32
#else
#include <io.h>
#endif
static void socket_once_init();


#ifndef WIN32


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
static void socket_once_init(void)
{
	mingw_ensure_socket_initialization();
}
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
		int fd_in;
		int fd_out;
		const char *argv[] = {
			"close-socket",
			NULL
		};
		if (fd_is_out)
		{
			fd_out = 1;
			fd_in = -1;
		}
		else
		{
			fd_out = -1;
			fd_in = 0;
			
		}
		memset(&cmd, 0, sizeof(cmd));
		cmd.argv = argv;
		cmd.in = fd_in;
		cmd.out = fd_out;
		cmd.git_cmd = 1;
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
	int socket_fd, int fdin, int fdout, int fderr, int git_cmd,
	struct child_process * cld)
{
	int result;
	logging_printf("start_command_socket_0\n");
	{
		env_buffer env_buf;
		char str_buffer[200];
		memset(&env_buf, 0, sizeof(env_buf));
		env_buffer_init(&env_buf, env);
		logging_printf("start_command_socket_0:1\n");
		snprintf(str_buffer, sizeof(str_buffer), 
			"_WIN_SOCK_IO={%d, %d}", fdin, fdout);
		env_buffer_add(&env_buf, str_buffer);
		env = (const char * const *)env_buffer_copy_env(&env_buf);
		env_buffer_close(&env_buf);
	}
	memset(cld, 0, sizeof(*cld));

	cld->env = (const char **)env;
	cld->argv = (const char **)argv;
	cld->in = -1;
	cld->out = 0;
	cld->err = fderr;
	cld->git_cmd = git_cmd;

	result = start_command(cld);
	if (result == 0) {
		WSAPROTOCOL_INFO pi;
		if (WSADuplicateSocket(fd_to_socket(socket_fd),
			cld->pid, &pi) == 0) {
			ssize_t ws;
			ws = xwrite(cld->in, &pi, sizeof(pi));
			if (ws == sizeof(pi)) {
				result = 0;
			}
			else {
				errno = SOCK_UTIL_PROTOCOL_ERROR;
				result = -1;
			}
			
		}
		else {
			result = -1;
			errno = WSAGetLastError();
		}
		if (result) {
			kill(cld->pid, SIGTERM);
			cld->pid = 0;
		}	

	}
	env_buffer_free_env((char **)env);

	return result;	
}
int start_command_socket_1(const char * const *argv,
	const char * const *env,
	int socket,  int fderr, int git_cmd,
	struct child_process * cld)
{
	return start_command_socket_0(argv, env, socket, 0, 1, fderr, git_cmd, 
	cld);
}

int 
setup_io_care_socket(void)
{
	int result;
	char *sock_info;
	sock_info = getenv("_WIN_SOCK_IO");	

	if (sock_info) {
		char *token;
		WSAPROTOCOL_INFO pi;
		SOCKET sock;
		int fd[2] = { -1, -1 };
		result = 0;
		sock_info = xstrdup(sock_info);	
		token = strpbrk(sock_info, " \t");
		token = strtok(sock_info, "{");

		if (token) {
			char *end_ptr;
			long val;
			token = strtok(token, ",");
			val = strtol(token, &end_ptr, 10);
			if (end_ptr != token) {
				fd[0] = (int)val;
			}
			token = strtok(NULL, "}");
			if (token) {
				val = strtol(token, &end_ptr, 10);
				if (end_ptr != token) {
					fd[1] = (int)val;
				}
			}
		}
		free(sock_info);
		{
			ssize_t rs;
			rs = xread(0, &pi, sizeof(pi));
			if (rs == sizeof(pi)) {
				result = 0;
			}
			else {
				result = -1;
				errno = SOCK_UTIL_PROTOCOL_ERROR;
			}
		}
		if (result == 0) {
			socket_once_init();
			sock = WSASocket(FROM_PROTOCOL_INFO, 
				FROM_PROTOCOL_INFO, FROM_PROTOCOL_INFO, 
				&pi, 0, 0);
			if (sock == INVALID_SOCKET) {
				errno = WSAGetLastError();	
				result = -1;
			}
		} else {
			sock = INVALID_SOCKET;
		}
		
		if (result == 0) {
			int sock_fd = _open_osfhandle(sock, 0);	
			if (fd[0] != -1) {
				dup2(sock_fd, fd[0]);
			}
			if (fd[1] != -1) {
				dup2(sock_fd, fd[1]);
			}
			close(sock_fd);
		} else {
			logging_printf("can't create socket: %d\n", 
				errno);

		}
	}
	else
	{
		result = 0;	
	}
	return result;
}

const char * socket_utils_strerror_0(int errnum)
{
	const static struct 
	{
		int number;
		const char *message;
	} number_message[] = {
		{
			SOCK_UTIL_PROTOCOL_ERROR,
			"protocol error"
		}
	};
	int i;
	const char *result;
	result = NULL;
	for (i = 0; i < sizeof(number_message) / sizeof(number_message[0]);
		i++) {
		if (errnum == number_message[i].number) {
			result = number_message[i].message;
			break;
		}
	}
	return result;
}
const char * socket_utils_strerror_1(int errnum)
{
	static char message_buffer[1024];
	int state;
	const char *result;
	state = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errnum, 0, 
		message_buffer, sizeof(message_buffer), NULL);
	result = state ? message_buffer : NULL;
	return result;
}


const char * socket_utils_strerror(int errnum)
{
	const char *(*strerror_funcs[])(int) = {
		socket_utils_strerror_0,
		socket_utils_strerror_1
	};
	int i;
	const char *result;
	result = NULL;
	for (i = 0; i < sizeof(strerror_funcs) / sizeof(strerror_funcs[0]);
		i++) {
		result = strerror_funcs[i](errnum);
		if (result)
		{
			break;
		}
	}
	return result;
}


