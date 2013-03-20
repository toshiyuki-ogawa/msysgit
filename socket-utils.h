#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H
#include "run-command.h"
#ifdef WIN32
#define fd_to_socket(fd) ((SOCKET)_get_osfhandle(fd))
#else
#define fd_to_socket(fd) (fd)
#endif
#define SOCK_UTIL_PROTOCOL_ERROR -100


extern const char * socket_utils_strerror(int errnum);

extern int is_socket(int fd);

extern void set_socket_to_time_wait(int fd, int fd_is_out);

extern int start_command_socket_0(const char * const *argv,
	const char * const *env,
	int socket, int fdin, int fdout, int fderr, int git_cmd,
	struct child_process * cld);

extern int start_command_socket_1(const char * const *argv,
	const char * const *env,
	int socket, int fderr, int git_cmd, 
	struct child_process * cld);

extern int setup_io_care_socket(void);
#endif
